// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ChunkWater.hpp>
#include <noggit/TileWater.hpp>
#include <noggit/liquid_layer.hpp>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>
#include <ClientFile.hpp>

ChunkWater::ChunkWater(MapChunk* chunk, TileWater* water_tile, float x, float z, bool use_mclq_green_lava)
  : xbase(x)
  , zbase(z)
  , vmin(x, 0.f, z)
  , vmax(x + CHUNKSIZE, 0.f, z + CHUNKSIZE)
  , _use_mclq_green_lava(use_mclq_green_lava)
  , _chunk(chunk)
  , _water_tile(water_tile)
{
}

void ChunkWater::from_mclq(std::vector<mclq>& layers)
{
  glm::vec3 pos(xbase, 0.0f, zbase);

  if (!Render.has_value()) Render.emplace();
  for (mclq& liquid : layers)
  {
    std::uint8_t mclq_liquid_type = 0;

    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        mclq_tile const& tile = liquid.tiles[z * 8 + x];

        misc::bit_or(Render.value().fishable, x, z, tile.fishable);
        misc::bit_or(Render.value().fatigue, x, z, tile.fatigue);

        if (!tile.dont_render)
        {
          mclq_liquid_type = tile.liquid_type;
        }
      }
    }

    switch (mclq_liquid_type)
    {
      case 1:
        _layers.emplace_back(this, pos, liquid, 2);
        break;
      case 3:
        _layers.emplace_back(this, pos, liquid, 4);
        break;
      case 4:
        _layers.emplace_back(this, pos, liquid, 1);
        break;
      case 6:
        _layers.emplace_back(this, pos, liquid, (_use_mclq_green_lava ? 15 : 3));

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
  if (header.ofsRenderMask)
  {
    Render.emplace();
    f.seek(basePos + header.ofsRenderMask);
    f.read(&Render.value(), sizeof(MH2O_Render));
  }

  for (std::size_t k = 0; k < header.nLayers; ++k)
  {
    MH2O_Information info;
    uint64_t infoMask = 0xFFFFFFFFFFFFFFFF; // default = all water

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
    _layers.emplace_back(this, f, basePos, pos, info, infoMask);
  }

  update_layers();
}


void ChunkWater::save(sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos)
{
  MH2O_Header header;

  // remove empty layers
  cleanup();

  if (hasData(0))
  {
    header.nLayers = _layers.size();

    if (Render.has_value())
    {
        header.ofsRenderMask = current_pos - base_pos;
        adt.Insert(current_pos, sizeof(MH2O_Render), reinterpret_cast<char*>(&Render.value()));
        current_pos += sizeof(MH2O_Render);
    }
    else
    {
        header.ofsRenderMask = 0;
    }

    header.ofsInformation = current_pos - base_pos;
    int info_pos = current_pos;

    std::size_t info_size = sizeof(MH2O_Information) * _layers.size();
    current_pos += info_size;

    adt.Extend(info_size);

    for (liquid_layer& layer : _layers)
    {
      layer.save(adt, base_pos, info_pos, current_pos);
    }
  }

  memcpy(adt.GetPointer<char>(header_pos), &header, sizeof(MH2O_Header));
  header_pos += sizeof(MH2O_Header);
}


void ChunkWater::autoGen(MapChunk *chunk, float factor)
{
  for (liquid_layer& layer : _layers)
  {
    layer.update_opacity(chunk, factor);
  }
  update_layers();
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

  for (liquid_layer& layer : _layers)
  {
    vmin.y = std::min (vmin.y, layer.min());
    vmax.y = std::max (vmax.y, layer.max());

    extents[0].y = std::min(extents[0].y, vmin.y);
    extents[1].y = std::max(extents[1].y, vmax.y);

    _water_tile->tagUpdate();
    count++;
  }

  _water_tile->tagExtents(true);

  vcenter = (vmin + vmax) * 0.5f;
}

bool ChunkWater::hasData(size_t layer) const
{
  return _layers.size() > layer;
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
  for (int i = _layers.size() - 1; i >= 0; --i)
  {
    if (_layers[i].empty())
    {
      _layers.erase(_layers.begin() + i);
      _water_tile->tagUpdate();
    }
  }
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

void ChunkWater::tagUpdate()
{
  _water_tile->tagUpdate();
}

