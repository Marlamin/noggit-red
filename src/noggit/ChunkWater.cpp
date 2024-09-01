// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/TileWater.hpp>
#include <noggit/liquid_layer.hpp>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>
#include <ClientFile.hpp>
#include <util/sExtendableArray.hpp>

#include <algorithm>

ChunkWater::ChunkWater(MapChunk* chunk, TileWater* water_tile, float x, float z, bool use_mclq_green_lava)
  : xbase(x)
  , zbase(z)
  , vmin(x, 0.f, z)
  , vmax(x + CHUNKSIZE, 0.f, z + CHUNKSIZE)
  , vcenter((vmin + vmax) * 0.5f)
  , _use_mclq_green_lava(use_mclq_green_lava)
  , _chunk(chunk)
  , _water_tile(water_tile)
{
}

void ChunkWater::from_mclq(std::vector<mclq>& layers)
{
  glm::vec3 pos(xbase, 0.0f, zbase);

  for (mclq& liquid : layers)
  {
    std::uint8_t mclq_liquid_type = 0;

    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        mclq_tile const& tile = liquid.tiles[z * 8 + x];

        misc::bit_or(attributes.fishable, x, z, tile.fishable);
        misc::bit_or(attributes.fatigue, x, z, tile.fatigue);

        if (!tile.dont_render)
        {
          mclq_liquid_type = tile.liquid_type;
        }
      }
    }

    switch (mclq_liquid_type)
    {
      case 1: // mclq ocean
        _layers.emplace_back(this, pos, liquid, LIQUID_OCEAN);
        break;
      case 3: // mclq slime
        _layers.emplace_back(this, pos, liquid, LIQUID_SLIME);
        break;
      case 4: // mclq river
        _layers.emplace_back(this, pos, liquid, LIQUID_WATER);
        break;
      case 6: // mclq magma
        _layers.emplace_back(this, pos, liquid, (_use_mclq_green_lava ? LIQUID_Green_Lava : LIQUID_MAGMA));

        break;
      default:
        LogError << "Invalid/unhandled MCLQ liquid type" << std::endl;
        break;
    }
    _water_tile->tagUpdate();
  }
  update_layers();
}

void ChunkWater::fromFile(BlizzardArchive::ClientFile &f, size_t basePos)
{
  MH2O_Header header;
  f.read(&header, sizeof(MH2O_Header));

  if (!header.nLayers)
  {
    return;
  }

  //render
  if (header.ofsAttributes)
  {
    f.seek(basePos + header.ofsAttributes);
    f.read(&attributes, sizeof(MH2O_Attributes));
  }
  else
  {
      // when chunks don't have attributes it means everything is set.
      attributes.fishable = 0xFFFFFFFFFFFFFFFF;
      attributes.fatigue = 0xFFFFFFFFFFFFFFFF;
  }

  for (std::size_t k = 0; k < header.nLayers; ++k)
  {
    MH2O_Information info;
    uint64_t infoMask = 0xFFFFFFFFFFFFFFFF; // default = all water. InfoMask + HeightMap combined

    //info
    f.seek(basePos + header.ofsInformation + sizeof(MH2O_Information)* k);
    f.read(&info, sizeof(MH2O_Information));

    //mask
    if (info.ofsInfoMask > 0 && info.height > 0)
    {
      size_t bitmask_size = static_cast<size_t>(std::ceil(info.height * info.width / 8.0f));

      f.seek(info.ofsInfoMask + basePos);
      // only read the relevant data
      f.read(&infoMask, bitmask_size);
    }

    glm::vec3 pos(xbase, 0.0f, zbase);
    _water_tile->tagUpdate();
    // liquid_layer(ChunkWater* chunk, BlizzardArchive::ClientFile& f, std::size_t base_pos, glm::vec3 const& base
    //              ,MH2O_Information const& info, std::uint64_t infomask);
    _layers.emplace_back(this, f, basePos, pos, info, infoMask);
  }

  update_layers();
}


void ChunkWater::save(util::sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos)
{
  MH2O_Header header;

  // remove empty layers
  cleanup();
  update_attributes();

  if (hasData(0))
  {
    header.nLayers = _layer_count;;

    // fagique only for single layer ocean chunk
    bool fatigue = _layers[0].has_fatigue();
    if (!fatigue)
    {
      header.ofsAttributes = current_pos - base_pos;
      adt.Insert(current_pos, sizeof(MH2O_Attributes), reinterpret_cast<char*>(&attributes));
      current_pos += sizeof(MH2O_Attributes);
    }
    else
    {
      header.ofsAttributes = 0;
    }

    header.ofsInformation = current_pos - base_pos;
    int info_pos = current_pos;

    std::size_t info_size = sizeof(MH2O_Information) * _layer_count;
    current_pos += static_cast<std::uint32_t>(info_size);

    adt.Extend(static_cast<long>(info_size));

    for (liquid_layer& layer : _layers)
    {
      layer.save(adt, base_pos, info_pos, current_pos);
    }
  }

  memcpy(adt.GetPointer<char>(header_pos).get(), &header, sizeof(MH2O_Header));
  header_pos += sizeof(MH2O_Header);
}

void ChunkWater::save_mclq(util::sExtendableArray& adt, int mcnk_pos, int& current_pos)
{
  // remove empty layers
  cleanup();
  update_attributes();

  if (hasData(0))
  {
    adt.Extend(sizeof(mclq) * _layer_count + 8);
    // size seems to be 0 in vanilla adts in the mclq chunk's header and set right in the mcnk header (layer_size * n_layer + 8)
    SetChunkHeader(adt, current_pos, 'MCLQ', 0);

    current_pos += 8;

    // it's possible to merge layers when they don't overlap (liquids using the same vertice, but at different height)
    // layer ordering seems to matter, having a lava layer then a river layer causes the lava layer to not render ingame
    // sorting order seems to be dependant on the flag ordering in the mcnk's header
    std::vector<std::pair<mclq, int>> mclq_layers;

    for (liquid_layer const& layer : _layers)
    {
      switch (layer.mclq_liquid_type())
      {
      case 6: // lava
        adt.GetPointer<MapChunkHeader>(mcnk_pos + 8)->flags.flags.lq_magma = 1;
        break;
      case 3: // slime
        adt.GetPointer<MapChunkHeader>(mcnk_pos + 8)->flags.flags.lq_slime = 1;
        break;
      case 1: // ocean
        adt.GetPointer<MapChunkHeader>(mcnk_pos + 8)->flags.flags.lq_ocean = 1;
        break;
      default: // river
        adt.GetPointer<MapChunkHeader>(mcnk_pos + 8)->flags.flags.lq_river = 1;
        break;
      }

      mclq_layers.push_back({ layer.to_mclq(attributes), layer.mclq_flag_ordering() });
    }

    auto cmp = [](std::pair<mclq, int> const& a, std::pair<mclq, int> const& b)
      {
        return a.second < b.second;
      };

    // sort the layers by flag order
    std::sort(mclq_layers.begin(), mclq_layers.end(), cmp);

    for (auto const& mclq_layer : mclq_layers)
    {
      std::memcpy(adt.GetPointer<char>(current_pos).get(), &mclq_layer.first, sizeof(mclq));
      current_pos += sizeof(mclq);
    }
  }
}


void ChunkWater::autoGen(MapChunk *chunk, float factor)
{
  for (liquid_layer& layer : _layers)
  {
    layer.update_opacity(chunk, factor);
  }
  update_layers();
}

void ChunkWater::update_underground_vertices_depth(MapChunk* chunk)
{
  for (liquid_layer& layer : _layers)
  {
    layer.update_underground_vertices_depth(chunk);
  }
}


void ChunkWater::CropWater(MapChunk* chunkTerrain)
{
  for (liquid_layer& layer : _layers)
  {
    layer.crop(chunkTerrain);
  }
  update_layers();
}

int ChunkWater::getType(size_t layer) const
{
  return hasData(layer) ? _layers[layer].liquidID() : 0;
}

void ChunkWater::setType(int type, size_t layer)
{
  if(hasData(layer))
  {
    _layers[layer].changeLiquidID(type);
  }
  _water_tile->tagUpdate();
}

bool ChunkWater::is_visible ( const float& cull_distance
                            , const math::frustum& frustum
                            , const glm::vec3& camera
                            , display_mode display
                            ) const
{
  static const float chunk_radius = std::sqrt (CHUNKSIZE * CHUNKSIZE / 2.0f);

  float dist = display == display_mode::in_3D
             ? (camera - vcenter).length() - chunk_radius
             : std::abs(camera.y - vmax.y);

  return frustum.intersects(vmax, vmin) && dist < cull_distance;
}

void ChunkWater::update_layers()
{
  std::size_t count = 0;

  auto& extents = _water_tile->getExtents();

  vmin.y = std::numeric_limits<float>::max();
  vmax.y = std::numeric_limits<float>::lowest();

  std::uint64_t debug_fishable_old = attributes.fishable;
  std::uint64_t debug_fatigue_old = attributes.fatigue;

  if (_auto_update_attributes)
  {
    // reset attributes
    attributes.fishable = 0;
    attributes.fatigue = 0;
  }

  for (liquid_layer& layer : _layers)
  {
    vmin.y = std::min (vmin.y, layer.min());
    vmax.y = std::max (vmax.y, layer.max());

    extents[0].y = std::min(extents[0].y, vmin.y);
    extents[1].y = std::max(extents[1].y, vmax.y);

    _water_tile->tagUpdate();

    if (_auto_update_attributes)
      layer.update_attributes(attributes);

    count++;
  }

  // some tests to compare with blizzard
  const bool run_tests = false;
  if (run_tests)
  {
    if (attributes.fishable != debug_fishable_old && _layers.size())
    {
      uint64_t x = attributes.fishable ^ debug_fishable_old; 
      int difference_count = 0;

      
      while (x > 0) {
          difference_count += x & 1;
          x >>= 1;
      }

      auto type = _layers.front().liquidID();
      std::vector<float> depths;

      int zero_depth_num = 0;
      int below_20_num = 0; // below 0.2
      for (auto& vert : _layers.front().getVertices())
      {
          depths.push_back(vert.depth);
          if (vert.depth == 0.0f)
              zero_depth_num++;
          if (vert.depth < 0.2f && vert.depth != 0.0f)
              below_20_num++;
      }
    }
  }


  _water_tile->tagExtents(true);

  vcenter = (vmin + vmax) * 0.5f;

  _layer_count = _layers.size();
}

bool ChunkWater::hasData(size_t layer) const
{
  return _layer_count > layer;
}


void ChunkWater::paintLiquid( glm::vec3 const& pos
                            , float radius
                            , int liquid_id
                            , bool add
                            , math::radians const& angle
                            , math::radians const& orientation
                            , bool lock
                            , glm::vec3 const& origin
                            , bool override_height
                            , bool override_liquid_id
                            , MapChunk* chunk
                            , float opacity_factor
                            )
{
  if (override_liquid_id && !override_height)
  {
    bool layer_found = false;
    for (liquid_layer& layer : _layers)
    {
      if (layer.liquidID() == liquid_id)
      {
        copy_height_to_layer(layer, pos, radius);
        layer_found = true;
        break;
      }
    }

    if (!layer_found)
    {
      liquid_layer layer(this, glm::vec3(xbase, 0.0f, zbase), pos.y, liquid_id);
      copy_height_to_layer(layer, pos, radius);
      _water_tile->tagUpdate();
      _layers.push_back(layer);
    }
  }

  bool painted = false;
  for (liquid_layer& layer : _layers)
  {
    // remove the water on all layers or paint the layer with selected id
    if (!add || layer.liquidID() == liquid_id || !override_liquid_id)
    {
      layer.paintLiquid(pos, radius, add, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
      painted = true;
    }
    else
    {
      layer.paintLiquid(pos, radius, false, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
    }
  }

  cleanup();

  if (!add || painted)
  {
    update_layers();
    return;
  }

  if (hasData(0))
  {
    liquid_layer layer(_layers[0]);
    layer.clear(); // remove the liquid to not override the other layer
    layer.paintLiquid(pos, radius, true, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
    layer.changeLiquidID(liquid_id);
    _water_tile->tagUpdate();
    _layers.push_back(layer);
  }
  else
  {
    liquid_layer layer(this, glm::vec3(xbase, 0.0f, zbase), pos.y, liquid_id);
    layer.paintLiquid(pos, radius, true, angle, orientation, lock, origin, override_height, chunk, opacity_factor);
    _water_tile->tagUpdate();
    _layers.push_back(layer);
  }

  update_layers();
}

void ChunkWater::cleanup()
{
  for (int i = static_cast<int>(_layer_count - 1); i >= 0; --i)
  {
    if (_layers[i].empty())
    {
      _layers.erase(_layers.begin() + i);
      _water_tile->tagUpdate();
    }
  }
  update_layers();
}

void ChunkWater::copy_height_to_layer(liquid_layer& target, glm::vec3 const& pos, float radius)
{
  for (liquid_layer& layer : _layers)
  {
    if (layer.liquidID() == target.liquidID())
    {
      continue;
    }

    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        if (misc::getShortestDist(pos.x, pos.z, xbase + x*UNITSIZE, zbase + z*UNITSIZE, UNITSIZE) <= radius)
        {
          if (layer.hasSubchunk(x, z))
          {
            target.copy_subchunk_height(x, z, layer);
          }
        }
      }
    }
  }
}

void ChunkWater::update_attributes()
{
  attributes.fishable = 0;
  attributes.fatigue = 0;

  for (liquid_layer& layer : _layers)
  {
      layer.update_attributes(attributes);
  }
}

void ChunkWater::tagUpdate()
{
  _water_tile->tagUpdate();
}

