// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Brush.h>
#include <noggit/Log.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <noggit/texture_set.hpp>
#include <ClientFile.hpp>
#include <algorithm>    // std::min
#include <array>
#include <sstream>

TextureSet::TextureSet (MapChunk* chunk, BlizzardArchive::ClientFile* f, size_t base
                        , bool use_big_alphamaps, bool do_not_fix_alpha_map, bool do_not_convert_alphamaps
                        , Noggit::NoggitRenderContext context, MapChunkHeader const& header)
  : nTextures(header.nLayers)
  , _do_not_convert_alphamaps(do_not_convert_alphamaps)
  , _context(context)
  , _chunk(chunk)
{

  std::copy(header.doodadMapping, header.doodadMapping + 8, _doodadMapping.begin());
  std::copy(header.doodadStencil, header.doodadStencil + 8, _doodadStencil.begin());

  if (nTextures)
  {
    f->seek(base + header.ofsLayer + 8);

    ENTRY_MCLY tmp_entry_mcly[4];

    for (size_t i = 0; i<nTextures; ++i)
    {
      f->read (&tmp_entry_mcly[i], sizeof(ENTRY_MCLY)); // f->read (&_layers_info[i], sizeof(ENTRY_MCLY));

      std::string const& texturefilename = chunk->mt->mTextureFilenames[tmp_entry_mcly[i].textureID];
      textures.emplace_back (texturefilename, _context);

      if (chunk->mt->_mtxf_entries.contains(texturefilename))
      {
          if (chunk->mt->_mtxf_entries[texturefilename].use_cubemap)
              textures.back().use_cubemap = true;
      }

      _layers_info[i].effectID = tmp_entry_mcly[i].effectID;
      _layers_info[i].flags = tmp_entry_mcly[i].flags;
    }

    size_t alpha_base = base + header.ofsAlpha + 8;

    for (unsigned int layer = 0; layer < nTextures; ++layer)
    {
      if (_layers_info[layer].flags & FLAG_USE_ALPHA)
      {
        f->seek (alpha_base + tmp_entry_mcly[layer].ofsAlpha);
        alphamaps[layer - 1] = std::make_unique<Alphamap>(f, _layers_info[layer].flags, use_big_alphamaps, do_not_fix_alpha_map);
      }
    }

    // always use big alpha for editing / rendering
    if (!use_big_alphamaps && !_do_not_convert_alphamaps)
    {
      convertToBigAlpha();
    }

    _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  }
}

int TextureSet::addTexture (scoped_blp_texture_reference texture)
{
  int texLevel = -1;

  if (nTextures < 4U)
  {
    texLevel = static_cast<int>(nTextures);
    nTextures++;

    textures.emplace_back (std::move (texture));
    _layers_info[texLevel] = layer_info();

    if (texLevel)
    {
        alphamaps[texLevel - 1] = std::make_unique<Alphamap>();
    }

    if (tmp_edit_values && nTextures == 1)
    {
      tmp_edit_values->map[0].fill(255.f);
    }
  }

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;

  return texLevel;
}

bool TextureSet::replace_texture (scoped_blp_texture_reference const& texture_to_replace, scoped_blp_texture_reference replacement_texture)
{
  int texture_to_replace_level = -1, replacement_texture_level = -1;

  for (int i = 0; i < nTextures; ++i)
  {
    if (textures[i] == texture_to_replace)
    {
      texture_to_replace_level = i;
    }
    else if (textures[i] == replacement_texture)
    {
      replacement_texture_level = i;
    }
  }

  if (texture_to_replace_level != -1)
  {
    textures[texture_to_replace_level] = std::move (replacement_texture);

    QSettings settings;
    bool do_merge_layers = settings.value("texture_merge_layers", true).toBool();

    // prevent texture duplication
    if (replacement_texture_level != -1 && replacement_texture_level != texture_to_replace_level)
    {
      if (!do_merge_layers)
      {
        auto sstream = std::stringstream();
        sstream << "error_" << replacement_texture_level << ".blp";

        std::string fallback_tex_name = sstream.str();
        auto fallback = scoped_blp_texture_reference(fallback_tex_name, _context);

        textures[replacement_texture_level] = std::move(fallback);
      }
      else
      {
        // temp alphamap changes are applied in here
        merge_layers(texture_to_replace_level, replacement_texture_level);
      }
    }
  }

  if (texture_to_replace_level != -1 || replacement_texture_level != -1)
      return true;
  else
      return false;
}

void TextureSet::swap_layers(int layer_1, int layer_2)
{
  int lower_texture_id = std::min(layer_1, layer_2);
  int upper_texture_id = std::max(layer_1, layer_2);

  if (lower_texture_id == upper_texture_id)
  {
    return;
  }

  if (lower_texture_id > upper_texture_id)
  {
    std::swap(lower_texture_id, upper_texture_id);
  }

  if (lower_texture_id >= 0 && upper_texture_id >= 0 && lower_texture_id < nTextures && upper_texture_id < nTextures)
  {
    apply_alpha_changes();

    std::swap(textures[lower_texture_id], textures[upper_texture_id]);
    std::swap(_layers_info[lower_texture_id], _layers_info[upper_texture_id]);

    int a1 = lower_texture_id - 1, a2 = upper_texture_id - 1;

    if (lower_texture_id)
    {
      std::swap(alphamaps[a1], alphamaps[a2]);
    }
    else
    {
      uint8_t alpha[4096];

      for (int i = 0; i < 4096; ++i)
      {
        alpha[i] = 255 - sum_alpha(i);
      }

      alphamaps[a2]->setAlpha(alpha);
    }

    _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
    _need_lod_texture_map_update = true;
  }
}

void TextureSet::eraseTextures()
{
  if (nTextures == 0)
  {
    return;
  }

  textures.clear();

  for (int i = 0; i < 4; ++i)
  {
    if (i > 0)
    {
      alphamaps[i - 1].reset();
    }
    _layers_info[i] = layer_info();
  }

  nTextures = 0;

  std::fill(_doodadMapping.begin(), _doodadMapping.end(), 0);

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;

  tmp_edit_values.reset();
}

void TextureSet::eraseTexture(size_t id)
{
  if (id >= nTextures)
  {
    return;
  }

  // shift textures above
  for (size_t i = id; i < nTextures - 1; i++)
  {
    if (i)
    {
      alphamaps[i - 1].reset();
      std::swap (alphamaps[i - 1], alphamaps[i]);
    }

    if (tmp_edit_values)
    {
      tmp_edit_values->map[i].swap(tmp_edit_values->map[i+1]);
    }

    _layers_info[i] = _layers_info[i + 1];
  }

  if (nTextures > 1)
  {
    alphamaps[nTextures - 2].reset();
  }

  nTextures--;
  textures.erase(textures.begin() + id);

  // erase the old info as a precaution but it's overriden when adding a new texture
  _layers_info[nTextures] = layer_info();

  // set the default values for the temporary alphamap too
  if (tmp_edit_values)
  {
    tmp_edit_values->map[nTextures].fill(0.f);
  }

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;
}

bool TextureSet::canPaintTexture(scoped_blp_texture_reference const& texture)
{
  if (nTextures)
  {
    for (size_t k = 0; k < nTextures; ++k)
    {
      if (textures[k] == texture)
      {
        return true;
      }
    }

    return nTextures < 4;
  }

  return true;
}

const std::string& TextureSet::filename(size_t id)
{
  return textures[id]->file_key().filepath();
}

bool TextureSet::eraseUnusedTextures()
{
  if (nTextures < 2)
  {
    return false;
  }

  std::set<int> visible_tex;

  if (tmp_edit_values)
  {
    auto& amaps = *tmp_edit_values;

    for (int layer = 0; layer < nTextures && visible_tex.size() < nTextures; ++layer)
    {
      for (int i = 0; i < 4096; ++i)
      {
        if (amaps[layer][i] > 0.f)
        {
          visible_tex.emplace(layer);
          break; // texture visible, go to the next layer
        }
      }
    }
  }
  else
  {
    for (int i = 0; i < 4096 && visible_tex.size() < nTextures; ++i)
    {
      uint8_t sum = 0;
      for (int n = 0; n < nTextures - 1; ++n)
      {
        uint8_t a = alphamaps[n]->getAlpha(i);
        sum += a;
        if (a > 0)
        {
          visible_tex.emplace(n + 1);
        }
      }

      // base layer visible
      if (sum < 255)
      {
        visible_tex.emplace(0);
      }
    }
  }

  if (visible_tex.size() < nTextures)
  {
    for (int i = static_cast<int>(nTextures) - 1; i >= 0; --i)
    {
      if (visible_tex.find(i) == visible_tex.end())
      {
        eraseTexture(i);
      }
    }

    _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
    _need_lod_texture_map_update = true;
    return true;
  }

  return false;
}

int TextureSet::texture_id(scoped_blp_texture_reference const& texture)
{
  for (int i = 0; i < nTextures; ++i)
  {
    if (textures[i] == texture)
    {
      return i;
    }
  }

  return -1;
}

int TextureSet::get_texture_index_or_add (scoped_blp_texture_reference texture, float target)
{
  for (int i = 0; i < nTextures; ++i)
  {
    if (textures[i] == texture)
    {
      return i;
    }
  }

  // don't add a texture for nothing
  if (target == 0)
  {
    return -1;
  }

  if (nTextures == 4 && !eraseUnusedTextures())
  {
    return -1;
  }

  return addTexture (std::move (texture));
}

std::array<std::array<std::uint8_t, 8>, 8> const TextureSet::getDoodadMappingReadable()
{
    std::array<std::array<std::uint8_t, 8>, 8> doodad_mapping{};

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            doodad_mapping[y][x] = getDoodadActiveLayerIdAt(x, y);
        }
    }
    return doodad_mapping;
}

uint8_t const TextureSet::getDoodadActiveLayerIdAt(unsigned int x, unsigned int y)
{
    if (x >= 8 || y >= 8)
        return 0; // not valid

    unsigned int firstbit_pos = x * 2;

    // bool first_bit = _doodadMapping[y] & (1 << firstbit_pos);
    // bool second_bit = _doodadMapping[y] & (1 << (firstbit_pos + 1) );
    // uint8_t layer_id = first_bit | (second_bit << 1); // first_bit + second_bit * 2;
    uint8_t layer_id = (_doodadMapping[y] >> firstbit_pos) & 0x03;

    return layer_id;
}

bool const TextureSet::getDoodadDisabledAt(int x, int y)
{
    if (x >= 8 || y >= 8)
        return true; // not valid. default to enabled

    // X and Y are swapped
    bool is_enabled = _doodadStencil[y] & (1 << (x));

    return is_enabled;
}

void TextureSet::setDetailDoodadsExclusion(float xbase, float zbase, glm::vec3 const& pos, float radius, bool big, bool add)
{
    // big = fill chunk
    if (big)
    {
        std::fill(_doodadStencil.begin(), _doodadStencil.end(), add ? 0xFF : 0x0);
    }
    else
    {
        for (int x = 0; x < 8; ++x)
        {
            for (int y = 0; y < 8; ++y)
            {
                if (misc::getShortestDist(pos.x, pos.z, xbase + (UNITSIZE * x),
                    zbase + (UNITSIZE * y), UNITSIZE) <= radius)
                {
                    int v = (1 << (x));

                    if (add)
                        _doodadStencil[y] |= v;
                    else
                        _doodadStencil[y] &= ~v;
                }

            }
        }
    }
    _chunk->registerChunkUpdate(ChunkUpdateFlags::DETAILDOODADS_EXCLUSION);
}

bool TextureSet::stampTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture, QImage* image, bool paint)
{

  bool changed = false;

  float zPos, xPos, dist, radius;

  int tex_layer = get_texture_index_or_add (std::move (texture), strength);

  if (tex_layer == -1 || nTextures == 1)
  {
    return nTextures == 1;
  }

  radius = brush->getRadius();

  create_temporary_alphamaps_if_needed();
  auto& amaps = *tmp_edit_values;

  zPos = zbase;

  for (int j = 0; j < 64; j++)
  {
    xPos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      if(std::abs(x - (xPos + TEXDETAILSIZE / 2.0)) > radius || std::abs(z - (zPos + TEXDETAILSIZE / 2.0f)) > radius)
      {
        xPos += TEXDETAILSIZE;
        continue;
      }

      dist = misc::dist(x, z, xPos + TEXDETAILSIZE / 2.0f, zPos + TEXDETAILSIZE / 2.0f);

      glm::vec3 const diff{glm::vec3{xPos + TEXDETAILSIZE / 2.0f, 0.f, zPos + TEXDETAILSIZE / 2.0f} - glm::vec3{x, 0.f, z}};

      int pixel_x = std::round(((diff.x + radius) / (2.f * radius)) * image->width());
      int pixel_y =  std::round(((diff.z + radius) / (2.f * radius)) * image->height());

      float image_factor;

      if (pixel_x >= 0 && pixel_x < image->width() && pixel_y >= 0 && pixel_y < image->height())
      {
        auto color = image->pixelColor(pixel_x, pixel_y);
        image_factor = (color.redF() + color.greenF() + color.blueF()) / 3.0f;
      }
      else
      {
        image_factor = 0;
      }

      std::size_t offset = i + 64 * j;
      // use double for more precision
      std::array<double,4> alpha_values;
      double total = 0.;

      for (int n = 0; n < 4; ++n)
      {
        total += alpha_values[n] = amaps[n][i + 64 * j];
      }

      double current_alpha = alpha_values[tex_layer];
      double sum_other_alphas = (total - current_alpha);
      double alpha_change = image_factor * (strength - current_alpha) * pressure;

      // alpha too low, set it to 0 directly
      if (alpha_change < 0. && current_alpha + alpha_change < 1.)
      {
        alpha_change = -current_alpha;
      }

      if (misc::float_equals(current_alpha, strength))
      {
        xPos += TEXDETAILSIZE;
        continue;
      }

      if (sum_other_alphas < 1.)
      {
        // alpha is currently at 254/255 -> set it at 255 and clear the rest of the values
        if (alpha_change > 0.f)
        {
          for (int layer = 0; layer < nTextures; ++layer)
          {
            alpha_values[layer] = layer == tex_layer ? 255. : 0.f;
          }
        }
          // all the other textures amount for less an 1/255 -> add the alpha_change (negative) to current texture and remove it from the first non current texture, clear the rest
        else
        {
          bool change_applied = false;

          for (int layer = 0; layer < nTextures; ++layer)
          {
            if (layer == tex_layer)
            {
              alpha_values[layer] += alpha_change;
            }
            else
            {
              if (!change_applied)
              {
                alpha_values[layer] -= alpha_change;
              }
              else
              {
                alpha_values[tex_layer] += alpha_values[layer];
                alpha_values[layer] = 0.;
              }

              change_applied = true;
            }
          }
        }
      }
      else
      {
        for (int layer = 0; layer < nTextures; ++layer)
        {
          if (layer == tex_layer)
          {
            alpha_values[layer] += alpha_change;
          }
          else
          {
            alpha_values[layer] -= alpha_change * alpha_values[layer] / sum_other_alphas;

            // clear values too low to be visible
            if (alpha_values[layer] < 1.)
            {
              alpha_values[tex_layer] += alpha_values[layer];
              alpha_values[layer] = 0.f;
            }
          }
        }
      }

      double total_final = std::accumulate(alpha_values.begin(), alpha_values.end(), 0.);

      // failsafe in case the sum of all alpha values deviate
      if (std::abs(total_final - 255.) > 0.001)
      {
        for (double& d : alpha_values)
        {
          d = d * 255. / total_final;
        }
      }

      for (int n = 0; n < 4; ++n)
      {
        amaps[n][i + 64 * j] = static_cast<float>(alpha_values[n]);
      }

      changed = true;


      xPos += TEXDETAILSIZE;
    }
    zPos += TEXDETAILSIZE;
  }

  if (!changed)
  {
    return false;
  }

  // cleanup
  eraseUnusedTextures();

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;

  return true;

  /*
  bool changed = false;

  float zPos, xPos, dist, radius;

  int tex_layer = get_texture_index_or_add (std::move (texture), strength);

  if (tex_layer == -1 || nTextures == 1)
  {
    return nTextures == 1;
  }

  radius = brush->getRadius();

  if (misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE) > radius)
  {
    return changed;
  }

  create_temporary_alphamaps_if_needed();
  auto& amaps = tmp_edit_values.get();

  zPos = zbase;

  for (int j = 0; j < 64; j++)
  {
    xPos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      if(std::abs(x - (xPos + TEXDETAILSIZE / 2.0)) > radius || std::abs(z - (zPos + TEXDETAILSIZE / 2.0f)) > radius)
        continue;

      dist = misc::dist(x, z, xPos + TEXDETAILSIZE / 2.0f, zPos + TEXDETAILSIZE / 2.0f);

      glm::vec3 const diff{glm::vec3{xPos + TEXDETAILSIZE / 2.0f, 0.f, zPos + TEXDETAILSIZE / 2.0f} - glm::vec3{x, 0.f, z}};

      int pixel_x = std::floor(diff.x + radius);
      int pixel_y = std::floor(diff.z + radius);

      auto color = image->pixelColor(pixel_x, pixel_y);
      float image_factor = (color.redF() + color.greenF() + color.blueF()) / 3.0f;

      std::size_t offset = i + 64 * j;

      float current_alpha = amaps[tex_layer][i + 64 * j];
      float sum_other_alphas = 1.f - current_alpha;
      float alpha_change = image_factor * ((strength - current_alpha) * pressure * brush->getValue(dist));

      if (misc::float_equals(current_alpha, strength))
        continue;

      float totOthers = 0.0f;
      for (int layer = 0; layer < nTextures; ++layer)
      {
        if (layer == tex_layer)
        {
          amaps[layer][offset] += alpha_change;
        }
        else
        {
          amaps[layer][offset] -= alpha_change * (amaps[layer][offset] / sum_other_alphas);
        }

        if (amaps[layer][offset] > 1.0f)
          amaps[layer][offset] = 1.0f;
        else if (amaps[layer][offset] < 0.0f || isnan(amaps[layer][offset]))
          amaps[layer][offset] = 0.0f;

        if(layer != 0)
          totOthers += amaps[layer][offset];
      }
      if (totOthers > 1)
      {
        float mul = (1 / totOthers);

        amaps[1][offset] = amaps[1][offset] * mul;
        totOthers = amaps[1][offset];
        if (nTextures > 2)
        {
          amaps[2][offset] = amaps[2][offset] * mul;
          totOthers += amaps[2][offset];
        }
        if (nTextures > 3)
        {
          amaps[3][offset] = amaps[3][offset] * mul;
          totOthers += amaps[3][offset];

        }
      }

      amaps[0][offset] = std::max(0.0f, std::min(1.0f - totOthers, 1.0f));
      changed = true;

      xPos += TEXDETAILSIZE;
    }
    zPos += TEXDETAILSIZE;
  }

  if (!changed)
  {
    return false;
  }

  // cleanup
  eraseUnusedTextures();

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;

  return true;

   */
}

bool TextureSet::paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  bool changed = false;

  float zPos, xPos, dist, radius;

  // todo: investigate the root cause
  // shift brush origin to avoid disconnects at the chunks' borders
  if (x < xbase)
  {
      float chunk_dist = std::abs(x - (std::fmod(x, CHUNKSIZE)) - xbase) / CHUNKSIZE;
      x += TEXDETAILSIZE * chunk_dist;
  }
  if (x > xbase + CHUNKSIZE)
  {
      float chunk_dist = std::abs(x - (std::fmod(x, CHUNKSIZE)) - xbase) / CHUNKSIZE;
      x -= TEXDETAILSIZE * chunk_dist;
  }
  if (z < zbase)
  {
      float chunk_dist = std::abs(z - (std::fmod(z, CHUNKSIZE)) - zbase) / CHUNKSIZE;
      z += TEXDETAILSIZE * chunk_dist;
  }
  if (z > zbase + CHUNKSIZE)
  {
      float chunk_dist = std::abs(z - (std::fmod(z, CHUNKSIZE)) - zbase) / CHUNKSIZE;
      z -= TEXDETAILSIZE * chunk_dist;
  }

  int tex_layer = get_texture_index_or_add (std::move (texture), strength);

  if (tex_layer == -1 || nTextures == 1)
  {
    return nTextures == 1;
  }

  radius = brush->getRadius();

  if (misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE) > radius)
  {
    return changed;
  }

  create_temporary_alphamaps_if_needed();
  auto& amaps = *tmp_edit_values;

  zPos = zbase;

  for (int j = 0; j < 64; j++)
  {
    xPos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      dist = misc::getShortestDist(x, z, xPos, zPos, TEXDETAILSIZE);

      if (dist <= radius)
      {
        std::size_t offset = i + 64 * j;
        // use double for more precision
        std::array<double,4> alpha_values;
        double total = 0.;

        for (int n = 0; n < 4; ++n)
        {
          total += alpha_values[n] = amaps[n][i + 64 * j];
        }

        double current_alpha = alpha_values[tex_layer];
        double sum_other_alphas = (total - current_alpha);
        double alpha_change = (strength - current_alpha) * pressure * brush->getValue(dist);

        // alpha too low, set it to 0 directly
        if (alpha_change < 0. && current_alpha + alpha_change < 1.)
        {
          alpha_change = -current_alpha;
        }

        if (!misc::float_equals(current_alpha, strength))
        {
          if (sum_other_alphas < 1.)
          {
            // alpha is currently at 254/255 -> set it at 255 and clear the rest of the values
            if (alpha_change > 0.f)
            {
              for (int layer = 0; layer < nTextures; ++layer)
              {
                alpha_values[layer] = layer == tex_layer ? 255. : 0.f;
              }
            }
            // all the other textures amount for less an 1/255 -> add the alpha_change (negative) to current texture and remove it from the first non current texture, clear the rest
            else
            {
              bool change_applied = false;

              for (int layer = 0; layer < nTextures; ++layer)
              {
                if (layer == tex_layer)
                {
                  alpha_values[layer] += alpha_change;
                }
                else
                {
                  if (!change_applied)
                  {
                    alpha_values[layer] -= alpha_change;
                  }
                  else
                  {
                    alpha_values[tex_layer] += alpha_values[layer];
                    alpha_values[layer] = 0.;
                  }

                  change_applied = true;
                }
              }
            }
          }
          else
          {
            for (int layer = 0; layer < nTextures; ++layer)
            {
              if (layer == tex_layer)
              {
                alpha_values[layer] += alpha_change;
              }
              else
              {
                alpha_values[layer] -= alpha_change * alpha_values[layer] / sum_other_alphas;

                // clear values too low to be visible
                if (alpha_values[layer] < 1.)
                {
                  alpha_values[tex_layer] += alpha_values[layer];
                  alpha_values[layer] = 0.f;
                }
              }
            }
          }

          double total_final = std::accumulate(alpha_values.begin(), alpha_values.end(), 0.);

          // failsafe in case the sum of all alpha values deviate
          if (std::abs(total_final - 255.) > 0.001)
          {
            for (double& d : alpha_values)
            {
              d = d * 255. / total_final;
            }
          }

          for (int n = 0; n < 4; ++n)
          {
            amaps[n][i + 64 * j] = static_cast<float>(alpha_values[n]);
          }

          changed = true;
        }
      }

      xPos += TEXDETAILSIZE;
    }
    zPos += TEXDETAILSIZE;
  }

  if (!changed)
  {
    return false;
  }

  // cleanup
  eraseUnusedTextures();

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;

  return true;
}

bool TextureSet::replace_texture( float xbase
                                , float zbase
                                , float x
                                , float z
                                , float radius
                                , scoped_blp_texture_reference const& texture_to_replace
                                , scoped_blp_texture_reference replacement_texture
                                , bool entire_chunk
                                )
{
  float dist = misc::getShortestDist(x, z, xbase, zbase, CHUNKSIZE);

  if (!entire_chunk && dist > radius)
  {
    return false;
  }

  if (entire_chunk)
  {
      replace_texture(texture_to_replace, std::move (replacement_texture));
      _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP);
      _need_lod_texture_map_update = true;
      return true;
  }

  // if the chunk is fully inside the brush, just swap the 2 textures
  if (misc::square_is_in_circle(x, z, radius, xbase, zbase, CHUNKSIZE))
  {
    replace_texture(texture_to_replace, std::move (replacement_texture));
    return true;
  }

  bool changed = false;
  int old_tex_level = -1, new_tex_level = -1;
  float x_pos, z_pos = zbase;

  for (int i=0; i<nTextures; ++i)
  {
    if (textures[i] == texture_to_replace)
    {
      old_tex_level = i;
    }
    if (textures[i] == replacement_texture)
    {
      new_tex_level = i;
    }
  }

  if (old_tex_level == -1 || (new_tex_level == -1 && nTextures == 4 && !eraseUnusedTextures()))
  {
    return false;
  }

  if (new_tex_level == -1)
  {
    new_tex_level = addTexture(std::move (replacement_texture));
  }

  if (old_tex_level == new_tex_level)
  {
    return false;
  }

  create_temporary_alphamaps_if_needed();
  auto& amap = *tmp_edit_values;

  for (int j = 0; j < 64; j++)
  {
    x_pos = xbase;
    for (int i = 0; i < 64; ++i)
    {
      dist = misc::dist(x, z, x_pos + TEXDETAILSIZE / 2.0f, z_pos + TEXDETAILSIZE / 2.0f);

      if (dist <= radius)
      {
        int offset = j * 64 + i;

        amap[new_tex_level][offset] += amap[old_tex_level][offset];
        amap[old_tex_level][offset] = 0.f;

        changed = true;
      }

      x_pos += TEXDETAILSIZE;
    }

    z_pos += TEXDETAILSIZE;
  }

  if (changed)
  {
    _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
    _need_lod_texture_map_update = true;
  }

  return changed;
}

unsigned int TextureSet::flag(size_t id)
{
  return _layers_info[id].flags;
}

unsigned int TextureSet::effect(size_t id)
{
  return _layers_info[id].effectID;
}

bool TextureSet::is_animated(std::size_t id) const
{
  return (id < nTextures ? (_layers_info[id].flags & FLAG_ANIMATE) : false);
}

void TextureSet::change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add)
{
  for (size_t i = 0; i < nTextures; ++i)
  {
    if (textures[i] == tex)
    {
      auto flag_view = reinterpret_cast<MCLYFlags*>(&_layers_info[i].flags);
      auto flag_view_new = reinterpret_cast<MCLYFlags*>(&flag);

      flag_view->animation_speed = flag_view_new->animation_speed;
      flag_view->animation_rotation = flag_view_new->animation_rotation;
      flag_view->animation_enabled = flag_view_new->animation_enabled;


      if (flag & FLAG_GLOW)
      {
        _layers_info[i].flags |= FLAG_GLOW;
      }
      else
      {
        _layers_info[i].flags &= ~FLAG_GLOW;
      }

      break;
    }
  }

  _chunk->registerChunkUpdate(ChunkUpdateFlags::FLAGS);
}

std::vector<std::vector<uint8_t>> TextureSet::save_alpha(bool big_alphamap)
{
  std::vector<std::vector<uint8_t>> amaps;

  apply_alpha_changes();

  if (nTextures > 1)
  {
    if (big_alphamap)
    {
      for (int i = 0; i < nTextures - 1; ++i)
      {
        const uint8_t* alphamap = alphamaps[i]->getAlpha();
        amaps.emplace_back(alphamap, alphamap + 4096);
      }
    }
    else
    {
      uint8_t tab[4096 * 3];

      if (_do_not_convert_alphamaps)
      {
        for (size_t k = 0; k < nTextures - 1; k++)
        {
          memcpy(tab + (k*64*64), alphamaps[k]->getAlpha(), 64 * 64);
        }
      }
      else
      {
        alphas_to_old_alpha(tab);
      }
      

      auto const combine_nibble
      (
        [&] (int layer, int pos)
        {
          int index = layer * 4096 + pos * 2;
          return ((tab[index] & 0xF0) >> 4) | (tab[index + 1] & 0xF0);
        }
      );

      for (int layer = 0; layer < nTextures - 1; ++layer)
      {
        amaps.emplace_back(2048);
        auto& layer_data = amaps.back();

        for (int i = 0; i < 2048; ++i)
        {
          layer_data[i] = combine_nibble(layer, i);
        }
      }
    }
  }

  return amaps;
}

scoped_blp_texture_reference TextureSet::texture(size_t id)
{
  return textures[id];
}

// dest = tab [4096 * (nTextures - 1)]
// call only if nTextures > 1
void TextureSet::alphas_to_big_alpha(uint8_t* dest)
{
  auto alpha
  (
    [&] (int layer, int pos = 0)
    {
      return dest + layer * 4096 + pos;
    }
  );

  for (int k = 0; k < nTextures - 1; k++)
  {
    memcpy(alpha(k), alphamaps[k]->getAlpha(), 4096);
  }

  for (int i = 0; i < 64 * 64; ++i)
  {
    int a = 255;

    for (int k = static_cast<int>(nTextures - 2); k >= 0; --k)
    {
      // uint8_t val = misc::rounded_255_int_div(*alpha(k, i) * a);
      uint8_t* value_ptr = alpha(k, i);
      uint8_t val = alpha_convertion_lookup[*value_ptr * a];
      a -= val;
      // *alpha(k, i) = val;
      *value_ptr = val;
    }
  }
}

void TextureSet::convertToBigAlpha()
{
  // nothing to do
  if (nTextures < 2)
  {
    return;
  }

  std::array<std::uint8_t, 4096 * 3> tab;

  apply_alpha_changes();
  alphas_to_big_alpha(tab.data());

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab.data() + 4096 * k);
  }
}

// dest = tab [4096 * (nTextures - 1)]
// call only if nTextures > 1
void TextureSet::alphas_to_old_alpha(uint8_t* dest)
{
  auto alpha
  (
    [&] (int layer, int pos = 0)
    {
      return dest + layer * 4096 + pos;
    }
  );

  for (int k = 0; k < nTextures - 1; k++)
  {
    memcpy(alpha(k), alphamaps[k]->getAlpha(), 64 * 64);
  }

  for (int i = 0; i < 64 * 64; ++i)
  {
    // a = remaining visibility
    int a = 255;

    for (int k = static_cast<int>(nTextures - 2); k >= 0; --k)
    {
      if (a <= 0)
      {
        *alpha(k, i) = 0;
      }
      else
      {
        int current = *alpha(k, i);
        // convert big alpha value to old alpha
        *alpha(k, i) = misc::rounded_int_div(current * 255, a);
        // remove big alpha value from the remaining visibility
        a -= current;
      }
    }
  }
}

void TextureSet::convertToOldAlpha()
{
  // nothing to do
  if (nTextures < 2)
  {
    return;
  }

  uint8_t tab[3 * 4096];

  apply_alpha_changes();
  alphas_to_old_alpha(tab);

  for (size_t k = 0; k < nTextures - 1; k++)
  {
    alphamaps[k]->setAlpha(tab + k * 4096);
  }

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
}

void TextureSet::merge_layers(size_t id1, size_t id2)
{
  if (id1 >= nTextures || id2 >= nTextures || id1 == id2)
  {
    throw std::invalid_argument("merge_layers: invalid layer id(s)");
  }

  if (id2 < id1)
  {
    std::swap(id2, id1);
  }

  create_temporary_alphamaps_if_needed();

  auto& amap = *tmp_edit_values;

  for (int i = 0; i < 64 * 64; ++i)
  {
    amap[id1][i] += amap[id2][i];
    // no need to set the id alphamap to 0, it'll be done in "eraseTexture(id2)"
  }

  eraseTexture(id2);
  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;
}

bool TextureSet::removeDuplicate()
{
  bool changed = apply_alpha_changes();

  for (size_t i = 0; i < nTextures; i++)
  {
    for (size_t j = i + 1; j < nTextures; j++)
    {
      if (textures[i] == textures[j])
      {
        merge_layers(i, j);
        changed = true;
        j--; // otherwise it skips the next texture
      }
    }
  }

  return changed;
}

void TextureSet::uploadAlphamapData()
{
  // This method assumes tile's alphamap storage is currently bound to the current texture unit

  if (!(_chunk->getUpdateFlags() & ChunkUpdateFlags::ALPHAMAP) || !nTextures)
    return;

  static std::array<float, 3 * 64 * 64> amap{};

  if (tmp_edit_values)
  {
    auto& tmp_amaps = *tmp_edit_values;

    for (int i = 0; i < 64 * 64; ++i)
    {
      for (int alpha_id = 0; alpha_id < 3; ++alpha_id)
      {
        amap[i * 3 + alpha_id] = (alpha_id < nTextures - 1) ? tmp_amaps[alpha_id + 1][i] / 255.f : 0.f;
      }
    }
  }
  else
  {
    uint8_t const* alpha_ptr[3];

    for (int i = 0; i < nTextures - 1; ++i)
    {
      alpha_ptr[i] = alphamaps[i]->getAlpha();
    }

    for (int i = 0; i < 64 * 64; ++i)
    {
      for (int alpha_id = 0; alpha_id < 3; ++alpha_id)
      {
          if (alpha_id < nTextures - 1)
          {
              amap[i * 3 + alpha_id] = *(alpha_ptr[alpha_id]++) / 255.0f;
          }
          else
          {
              amap[i * 3 + alpha_id] = 0.f;
          }
      }
    }

  }

  gl.texSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, _chunk->px * 16 + _chunk->py,
                   64, 64, 1, GL_RGB, GL_FLOAT, amap.data());

}

namespace
{
  misc::max_capacity_stack_vector<std::size_t, 4> current_layer_values
    (std::uint8_t nTextures, const std::array<std::unique_ptr<Alphamap>, 3>& alphamaps, std::size_t pz, std::size_t px)
  {
    misc::max_capacity_stack_vector<std::size_t, 4> values (nTextures, 0xFF);
    for (std::uint8_t i = 1; i < nTextures; ++i)
    {
      values[i] = alphamaps[i - 1]->getAlpha(64 * pz + px);
      values[0] -= values[i];
    }
    return values;
  }
}


void TextureSet::update_lod_texture_map()
{
  std::array<std::uint16_t, 8> lod{};

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      misc::max_capacity_stack_vector<std::size_t, 4> dominant_square_count (nTextures);

      for (int pz = z * 8; pz < (z + 1) * 8; ++pz)
      {
        for (int px = x * 8; px < (x + 1) * 8; ++px)
        {
          ++dominant_square_count[max_element_index (current_layer_values (static_cast<std::uint8_t>(nTextures), alphamaps, pz, px))];
        }
      }
      //lod.push_back (max_element_index (dominant_square_count));
    }
  }

  _doodadMapping = lod;
  _need_lod_texture_map_update = false;
}



std::array<float, 4> TextureSet::get_textures_weight_for_unit(unsigned int unit_x, unsigned int unit_y)
{
    float total_layer_0 = 0.f;
    float total_layer_1 = 0.f;
    float total_layer_2 = 0.f;
    float total_layer_3 = 0.f;

    if (nTextures == 0)
        return std::array<float, 4> { 0.f, 0.f, 0.f, 0.f };
    else if (nTextures == 1)
        return std::array<float, 4> { 100.f, 0.f, 0.f, 0.f };

    // 8x8 bits per unit
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            float base_alpha = 255.f;

            for (int alpha_layer = 0; alpha_layer < (nTextures - 1); ++alpha_layer)
            {
                float f = static_cast<float>(alphamaps[alpha_layer]->getAlpha( (unit_y * 8  + y)* 64 + (unit_x * 8 + x) )); // getAlpha(64 * y + x))

                if (alpha_layer == 0)
                    total_layer_1 += f;
                else if (alpha_layer == 1)
                    total_layer_2 += f;
                else if (alpha_layer == 2)
                    total_layer_3 += f;

                base_alpha -= f;
            }
            total_layer_0 += base_alpha;
        }
    }

    float sum = total_layer_0 + total_layer_1 + total_layer_2 + total_layer_3;
    std::array<float, 4> weights = { total_layer_0 / sum * 100.f,
        total_layer_1 / sum * 100.f,
        total_layer_2 / sum * 100.f,
        total_layer_3 / sum * 100.f };

    return weights;
}

void TextureSet::updateDoodadMapping()
{
    // NOTE : tempalphamap needs to be applied first with apply_alpha_changes()

    std::array<std::uint16_t, 8> new_doodad_mapping{};
    // std::array<std::array<std::uint8_t, 8>, 8> new_doodad_mapping{};
    // for (auto& row : new_doodad_mapping) {
    //     row.fill(nTextures - 1);
    // }

    if (nTextures <= 1)
    {
        // 0 or 1 layer, we just use the 0 default
        _doodadMapping = new_doodad_mapping;
        return;
    }

    // test comparison variables 
    int matching_count = 0;
    int not_matching_count = 0;
    int very_innacurate_count = 0;
    int higher_count = 0;
    int lower_count = 0;
    std::array<std::array<std::uint8_t, 8>, 8> blizzard_mapping_readable;
    bool debug_test = true;
    if (debug_test)
        blizzard_mapping_readable = getDoodadMappingReadable();

    // 8x8 bits per unit
    for (int unit_x = 0; unit_x < 8; unit_x++)
    {
        for (int unit_y = 0; unit_y < 8; unit_y++)
        {
            int layer_totals[4]{ 0,0,0,0 };

            // 8x8 bits per unit
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    unsigned int base_alpha = 255;

                    for (int alpha_layer = 0; alpha_layer < (nTextures - 1); ++alpha_layer)
                    {
                        auto alpha = static_cast<int>(alphamaps[alpha_layer]->getAlpha((unit_y * 8 + y) * 64 + (unit_x * 8 + x)));

                        layer_totals[alpha_layer+1] += alpha;

                        base_alpha -= alpha;
                    }
                    layer_totals[0] += base_alpha;
                }
            }

            // int sum = layer_totals[0] + layer_totals[1] + layer_totals[2] + layer_totals[3];
            // std::array<float, 4> percent_weights = { total_layer_0 / sum * 100.f,
            //     total_layer_1 / sum * 100.f,
            //     total_layer_2 / sum * 100.f,
            //     total_layer_3 / sum * 100.f };

            int max = layer_totals[0];
            int max_layer_index = 0;

            for (int i = 1; i < nTextures; i++)
            {
                // in old azeroth maps superior layers seems to have higher priority
                // error margin is ~4% in old azeroth without adjusted weight, 2% with
                // error margin is < 0.5% in northrend without adjusting

                float adjusted_weight = layer_totals[i] * (1 + 0.01*i); // superior layer seems to have priority, adjust by 1% per layer
                // if (layer_totals[i] >= max)
                // if (std::floor(weights[i]) >= max)
                // if (std::round(weights[i]) >= max)
                if (adjusted_weight >= max) // this with 5% works the best in old continents
                {
                    max = layer_totals[i];
                    max_layer_index = i;
                }
            }
            unsigned int firstbit_pos = unit_x * 2;
            new_doodad_mapping[unit_y] |= ((max_layer_index & 3) << firstbit_pos);
            // new_chunk_mapping[y][x] = max_layer_index;

            // debug compare with original data
            if (debug_test)
            {
                uint8_t blizzard_layer_id = blizzard_mapping_readable[unit_y][unit_x];
                uint8_t blizzard_layer_id2 = getDoodadActiveLayerIdAt(unit_x, unit_y); // make sure both work the same
                if (blizzard_layer_id != blizzard_layer_id2)
                    throw;
                // bool test_doodads_enabled = local_chunk->getTextureSet()->getDoodadDisabledAt(x, y);

                if (max_layer_index < blizzard_layer_id)
                    lower_count++;
                if (max_layer_index > blizzard_layer_id)
                    higher_count++;

                if (max_layer_index != blizzard_layer_id)
                {
                    int blizzard_effect_id = getEffectForLayer(blizzard_layer_id);
                    int found_effect_id = getEffectForLayer(max_layer_index);
                    not_matching_count++;
                    /*
                    float percent_innacuracy = ((layer_totals[max_layer_index] - layer_totals[blizzard_layer_id]) / ((static_cast<float>(layer_totals[max_layer_index]) + layer_totals[blizzard_layer_id]) / 2)) * 100.f;

                    if (percent_innacuracy > 15)
                        very_innacurate_count++;*/

                }
                else
                    matching_count++;
            }
        }
    }

    _doodadMapping = new_doodad_mapping;
}

uint8_t TextureSet::sum_alpha(size_t offset) const
{
  uint8_t sum = 0;

  for (auto const& amap : alphamaps)
  {
    if (amap)
    {
      sum += amap->getAlpha(offset);
    }
  }

  return sum;
}

namespace
{
  inline std::uint8_t float_alpha_to_uint8(float a)
  {
    return static_cast<std::uint8_t>(std::max(0.f, std::min(255.f, std::round(a))));
  }
}

bool TextureSet::apply_alpha_changes()
{
  if (!tmp_edit_values || nTextures < 2)
  {
    tmp_edit_values.reset();
    return false;
  }

  auto& new_amaps = *tmp_edit_values;
  std::array<std::uint16_t, 64 * 64> totals;
  totals.fill(0);

  for (int alpha_layer = 0; alpha_layer < nTextures - 1; ++alpha_layer)
  {
    std::array<std::uint8_t, 64 * 64> values;

    for (int i = 0; i < 64 * 64; ++i)
    {
      values[i] = float_alpha_to_uint8(new_amaps[alpha_layer + 1][i]);
      totals[i] += values[i];

      // remove the possible overflow with rounding
      // max 2 if all 4 values round up so it won't change the layer's alpha much
      if (totals[i] > 255)
      {
        values[i] -= static_cast<std::uint8_t>(totals[i] - 255);
      }
    }

    alphamaps[alpha_layer]->setAlpha(values.data());
  }

  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true;

  tmp_edit_values.reset();

  return true;
}

void TextureSet::create_temporary_alphamaps_if_needed()
{
  if (tmp_edit_values || nTextures < 2)
  {
    return;
  }

  tmp_edit_values = std::make_unique<tmp_edit_alpha_values>();

  tmp_edit_alpha_values& values = *tmp_edit_values;

  for (int i = 0; i < 64 * 64; ++i)
  {
    float base_alpha = 255.f;

    for (int alpha_layer = 0; alpha_layer < nTextures - 1; ++alpha_layer)
    {
      float f = static_cast<float>(alphamaps[alpha_layer]->getAlpha(i));

      values[alpha_layer + 1][i] = f;
      base_alpha -= f;
    }

    values[0][i] = base_alpha;
  }
}

void TextureSet::markDirty()
{
  _chunk->registerChunkUpdate(ChunkUpdateFlags::ALPHAMAP); 
  _need_lod_texture_map_update = true; 
}

void TextureSet::setEffect(size_t id, int value)
{
  _layers_info[id].effectID = value;
  _chunk->registerChunkUpdate(ChunkUpdateFlags::FLAGS);
}

std::array<std::uint16_t, 8> TextureSet::lod_texture_map()
{
  // make sure all changes have been applied
  apply_alpha_changes();

  if (_need_lod_texture_map_update)
  {
    update_lod_texture_map();
  }

  return _doodadMapping;
}

std::array<std::uint8_t, 256 * 256> TextureSet::alpha_convertion_lookup = TextureSet::make_alpha_lookup_array();