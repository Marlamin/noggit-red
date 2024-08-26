// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>
#include <ClientFile.hpp>

#include <algorithm>
#include <string>

namespace
{
  inline glm::vec2 default_uv(int px, int pz)
  {
    return {static_cast<float>(px) / 4.f, static_cast<float>(pz) / 4.f};
  }
}

liquid_layer::liquid_layer(ChunkWater* chunk, glm::vec3 const& base, float height, int liquid_id)
  : _liquid_id(liquid_id)
  , _liquid_vertex_format(HEIGHT_DEPTH)
  , _minimum(height)
  , _maximum(height)
  , _subchunks(0)
  , pos(base)
  , _chunk(chunk)
{
  if (!gLiquidTypeDB.CheckIfIdExists(_liquid_id))
    _liquid_id = LIQUID_WATER;

  create_vertices(height);

  changeLiquidID(_liquid_id);
  
}

liquid_layer::liquid_layer(ChunkWater* chunk, glm::vec3 const& base, mclq& liquid, int liquid_id)
  : _liquid_id(liquid_id)
  , _minimum(liquid.min_height)
  , _maximum(liquid.max_height)
  , _subchunks(0)
  , pos(base)
  , _chunk(chunk)
{
  if (!gLiquidTypeDB.CheckIfIdExists(_liquid_id))
    _liquid_id = LIQUID_WATER;

  changeLiquidID(_liquid_id);

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      misc::set_bit(_subchunks, x, z, !liquid.tiles[z * 8 + x].dont_render);
    }
  }

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      const unsigned v_index = z * 9 + x;
      mclq_vertex const& v = liquid.vertices[v_index];

      liquid_vertex lv;

      // _liquid_vertex_format is set by changeLiquidID()
      if (_liquid_vertex_format == HEIGHT_UV)
      {
        lv.depth = 1.f;
        lv.uv = { static_cast<float>(v.magma.x) / 255.f, static_cast<float>(v.magma.y) / 255.f };
      }
      else
      {
        lv.depth = static_cast<float>(v.water.depth) / 255.f;
        lv.uv = default_uv(x, z);
      }

      // sometimes there's garbage data on unused tiles that mess things up
      lv.position = { pos.x + UNITSIZE * x, std::clamp(v.height, _minimum, _maximum), pos.z + UNITSIZE * z };


      _vertices[v_index] = lv;
    }
  }

}

liquid_layer::liquid_layer(ChunkWater* chunk
                           , BlizzardArchive::ClientFile& f
                           , std::size_t base_pos
                           , glm::vec3 const& base
                           , MH2O_Information const& info
                           , std::uint64_t infomask)
  : _liquid_id(info.liquid_id)
  , _liquid_vertex_format(info.liquid_vertex_format)
  , _minimum(info.minHeight)
  , _maximum(info.maxHeight)
  , _subchunks(0)
  , pos(base)
  , _chunk(chunk)
{
  // check if liquid id is valid or some downported maps will crash
  if (!gLiquidTypeDB.CheckIfIdExists(_liquid_id))
    _liquid_id = LIQUID_WATER;

  int offset = 0;
  for (int z = 0; z < info.height; ++z)
  {
    for (int x = 0; x < info.width; ++x)
    {
      setSubchunk(x + info.xOffset, z + info.yOffset, (infomask >> offset) & 1);
      offset++;
    }
  }

  // default values
  create_vertices(_minimum);

  if (info.ofsHeightMap)
  {
    f.seek(base_pos + info.ofsHeightMap);

    if (_liquid_vertex_format == HEIGHT_DEPTH || _liquid_vertex_format == HEIGHT_UV)
    {

      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
            float h;
            f.read(&h, sizeof(float));

            _vertices[z * 9 + x].position.y = std::clamp(h, _minimum, _maximum);
        }
      }
    }

    if (_liquid_vertex_format == HEIGHT_UV)
    {
      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          mh2o_uv uv;
          f.read(&uv, sizeof(mh2o_uv));
          _vertices[z * 9 + x].uv =
            { static_cast<float>(uv.x) / 255.f
            , static_cast<float>(uv.y) / 255.f
            };
        }
      }
    }

    if (_liquid_vertex_format == HEIGHT_DEPTH || _liquid_vertex_format == DEPTH)
    {
      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          std::uint8_t depth;
          f.read(&depth, sizeof(std::uint8_t));
          _vertices[z * 9 + x].depth = static_cast<float>(depth) / 255.f;
        }
      }
    }
  }

  changeLiquidID(_liquid_id); // to update the liquid type

  if (check_fatigue())
    _fatigue_enabled = true;
}

liquid_layer::liquid_layer(liquid_layer&& other)
  : _liquid_id(other._liquid_id)
  , _liquid_vertex_format(other._liquid_vertex_format)
  , _minimum(other._minimum)
  , _maximum(other._maximum)
  // , _center(other._center)
  , _subchunks(other._subchunks)
  , _vertices(other._vertices)
  // , _indices_by_lod(other._indices_by_lod)
  , _fatigue_enabled(other._fatigue_enabled)
  , pos(other.pos)
  , _chunk(other._chunk)
{
  // update liquid type and vertex format
  changeLiquidID(_liquid_id);
}

liquid_layer::liquid_layer(liquid_layer const& other)
  : _liquid_id(other._liquid_id)
  , _liquid_vertex_format(other._liquid_vertex_format)
  , _minimum(other._minimum)
  , _maximum(other._maximum)
  , _subchunks(other._subchunks)
  , _vertices(other._vertices)
  // , _indices_by_lod(other._indices_by_lod)
  , _fatigue_enabled(other._fatigue_enabled)
  , pos(other.pos)
  , _chunk(other._chunk)
{
  // update liquid type and vertex format
  changeLiquidID(_liquid_id);
}

liquid_layer& liquid_layer::operator= (liquid_layer&& other)
{
  std::swap(_liquid_id, other._liquid_id);
  std::swap(_liquid_vertex_format, other._liquid_vertex_format);
  std::swap(_minimum, other._minimum);
  std::swap(_maximum, other._maximum);
  std::swap(_subchunks, other._subchunks);
  std::swap(_vertices, other._vertices);
  std::swap(_fatigue_enabled, other._fatigue_enabled);
  std::swap(pos, other.pos);
  // std::swap(_indices_by_lod, other._indices_by_lod);
  std::swap(_chunk, other._chunk);

  // update liquid type and vertex format
  changeLiquidID(_liquid_id);
  other.changeLiquidID(other._liquid_id);

  return *this;
}

liquid_layer& liquid_layer::operator=(liquid_layer const& other)
{

  _liquid_vertex_format = other._liquid_vertex_format;
  _minimum = other._minimum;
  _maximum = other._maximum;
  _subchunks = other._subchunks;
  _vertices = other._vertices;
  pos = other.pos;
  // _indices_by_lod = other._indices_by_lod;
  _fatigue_enabled = other._fatigue_enabled;
  _chunk = other._chunk;

  // update liquid type and vertex format
  changeLiquidID(other._liquid_id);
  return *this;
}

void liquid_layer::create_vertices(float height)
{
    int index = 0;
    for (int z = 0; z < 9; ++z)
    {
        const float posZ = pos.z + UNITSIZE * z;
        for (int x = 0; x < 9; ++x, ++index)
        {
            _vertices[index] = liquid_vertex( glm::vec3(pos.x + UNITSIZE * x, height, posZ)
                , default_uv(x, z)
                , 1.f
            );
        }
    }
}

void liquid_layer::save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const
{
  int min_x = 9, min_z = 9, max_x = 0, max_z = 0;
  bool filled = true;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (hasSubchunk(x, z))
      {
        min_x = std::min(x, min_x);
        min_z = std::min(z, min_z);
        max_x = std::max(x + 1, max_x);
        max_z = std::max(z + 1, max_z);
      }
      else
      {
        filled = false;
      }
    }
  }

  MH2O_Information info;
  std::uint64_t mask = 0;

  info.liquid_id = _liquid_id;
  info.liquid_vertex_format = _liquid_vertex_format;
  info.minHeight = _minimum;
  info.maxHeight = _maximum;
  info.xOffset = min_x;
  info.yOffset = min_z;
  info.width = max_x - min_x;
  info.height = max_z - min_z;

  if (filled)
  {
    info.ofsInfoMask = 0;
  }
  else
  {
    std::uint64_t value = 1;
    for (int z = info.yOffset; z < info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x < info.xOffset + info.width; ++x)
      {
        if (hasSubchunk(x, z))
        {
          mask |= value;
        }
        value <<= 1;
      }
    }

    if (mask > 0)
    {
      info.ofsInfoMask = current_pos - base_pos;
      adt.Insert(current_pos, 8, reinterpret_cast<char*>(&mask));
      current_pos += 8;
    }
  }

  int vertices_count = (info.width + 1) * (info.height + 1);
  info.ofsHeightMap = current_pos - base_pos;

  if (_liquid_vertex_format == HEIGHT_DEPTH || _liquid_vertex_format == HEIGHT_UV)
  {
    adt.Extend(vertices_count * sizeof(float));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        memcpy(adt.GetPointer<char>(current_pos), &_vertices[z * 9 + x].position.y, sizeof(float));
        current_pos += sizeof(float);
      }
    }
  }
  // no heightmap/depth data for fatigue chunks
  else if (_fatigue_enabled)
  {
      info.ofsHeightMap = 0;
  }

  if (_liquid_vertex_format == HEIGHT_UV)
  {
    adt.Extend(vertices_count * sizeof(mh2o_uv));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        mh2o_uv uv;
        uv.x = static_cast<std::uint16_t>(std::min(_vertices[z * 9 + x].uv.x * 255.f, 65535.f));
        uv.y = static_cast<std::uint16_t>(std::min(_vertices[z * 9 + x].uv.y * 255.f, 65535.f));

        memcpy(adt.GetPointer<char>(current_pos), &uv, sizeof(mh2o_uv));
        current_pos += sizeof(mh2o_uv);
      }
    }
  }

  if (_liquid_vertex_format == HEIGHT_DEPTH || (_liquid_vertex_format == DEPTH && !_fatigue_enabled))
  {
    adt.Extend(vertices_count * sizeof(std::uint8_t));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
          std::uint8_t depth = static_cast<std::uint8_t>(std::min(_vertices[z * 9 + x].depth * 255.0f, 255.f));
        memcpy(adt.GetPointer<char>(current_pos), &depth, sizeof(std::uint8_t));
        current_pos += sizeof(std::uint8_t);
      }
    }
  }

  memcpy(adt.GetPointer<char>(info_pos), &info, sizeof(MH2O_Information));
  info_pos += sizeof(MH2O_Information);
}

void liquid_layer::changeLiquidID(int id)
{
  _liquid_id = id;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(_liquid_id);

    _liquid_type = lLiquidTypeRow.getInt(LiquidTypeDB::Type);

    switch (_liquid_type)
    {
    case liquid_basic_types_magma:
    case liquid_basic_types_slime:
      _liquid_vertex_format = HEIGHT_UV;
      break;
    case liquid_basic_types_ocean: // ocean
      _liquid_vertex_format = DEPTH;
      break;
    default:
      _liquid_vertex_format = HEIGHT_DEPTH;
      break;
    }
  }
  catch (LiquidTypeDB::NotFound)
  {
      assert(false);
      LogError << "Liquid type id " << _liquid_type << " not found in LiquidType dbc" << std::endl;
  }
}

void liquid_layer::crop(MapChunk* chunk)
{
  if (_maximum < chunk->getMinHeight())
  {
    _subchunks = 0;
  }
  else
  {
    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        if (hasSubchunk(x, z))
        {
          int water_index = 9 * z + x, terrain_index = 17 * z + x;

          if ( _vertices[water_index +  0].position.y < chunk->mVertices[terrain_index +  0].y
            && _vertices[water_index +  1].position.y < chunk->mVertices[terrain_index +  1].y
            && _vertices[water_index +  9].position.y < chunk->mVertices[terrain_index + 17].y
            && _vertices[water_index + 10].position.y < chunk->mVertices[terrain_index + 18].y
            )
          {
            setSubchunk(x, z, false);
          }
        }
      }
    }
  }

  update_min_max();
}

void liquid_layer::update_opacity(MapChunk* chunk, float factor)
{
  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      update_vertex_opacity(x, z, chunk, factor);
    }
  }
}

bool liquid_layer::hasSubchunk(int x, int z, int size) const
{
  for (int pz = z; pz < z + size; ++pz)
  {
    for (int px = x; px < x + size; ++px)
    {
      if ((_subchunks >> (pz * 8 + px)) & 1)
      {
        return true;
      }
    }
  }
  return false;
}

void liquid_layer::setSubchunk(int x, int z, bool water)
{
  misc::set_bit(_subchunks, x, z, water);
}

void liquid_layer::paintLiquid( glm::vec3 const& cursor_pos
                              , float radius
                              , bool add
                              , math::radians const& angle
                              , math::radians const& orientation
                              , bool lock
                              , glm::vec3 const& origin
                              , bool override_height
                              , MapChunk* chunk
                              , float opacity_factor
                              )
{
  glm::vec3 ref ( lock
                      ? origin
                      : glm::vec3 (cursor_pos.x, cursor_pos.y + 1.0f, cursor_pos.z)
                      );

  int id = 0;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (misc::getShortestDist(cursor_pos, _vertices[id].position, UNITSIZE) <= radius)
      {
        if (add)
        {
          for (int index : {id, id + 1, id + 9, id + 10})
          {
            bool no_subchunk = !hasSubchunk(x, z);
            bool in_range = misc::dist(cursor_pos, _vertices[index].position) <= radius;

            if (no_subchunk || (in_range && override_height))
            {
              _vertices[index].position.y = misc::angledHeight(ref, _vertices[index].position, angle, orientation);
            }
            if (no_subchunk || in_range)
            {
              update_vertex_opacity(index % 9, index / 9, chunk, opacity_factor);
            }
          }
        }
        setSubchunk(x, z, add);
      }

      id++;
    }
    // to go to the next row of subchunks
    id++;
  }

  update_min_max();
}

void liquid_layer::update_min_max()
{
  _minimum = std::numeric_limits<float>::max();
  _maximum = std::numeric_limits<float>::lowest();
  int x = 0, z = 0;

  for (liquid_vertex& v : _vertices)
  {
    if (hasSubchunk(std::min(x, 7), std::min(z, 7)))
    {
      _maximum = std::max(_maximum, v.position.y);
      _minimum = std::min(_minimum, v.position.y);
    }

    if (++x == 9)
    {
      z++;
      x = 0;
    }
  }

  // lvf = 2 means the liquid height is 0, switch to lvf 0 when that's not the case
  if (_liquid_vertex_format == DEPTH && (!misc::float_equals(0.f, _minimum) || !misc::float_equals(0.f, _maximum)))
  {
    _liquid_vertex_format = HEIGHT_DEPTH;
  }
  // use lvf 2 when possible to save space
  else if (_liquid_vertex_format == HEIGHT_DEPTH && misc::float_equals(0.f, _minimum) && misc::float_equals(0.f, _maximum))
  {
    _liquid_vertex_format = DEPTH;
  }

  _fatigue_enabled = check_fatigue();
  // recalc all atributes instead?
  // _chunk->update_layers();
}

void liquid_layer::copy_subchunk_height(int x, int z, liquid_layer const& from)
{
  int id = 9 * z + x;

  for (int index : {id, id + 1, id + 9, id + 10})
  {
    _vertices[index].position.y = from._vertices[index].position.y;
  }

  setSubchunk(x, z, true);
}

void liquid_layer::update_vertex_opacity(int x, int z, MapChunk* chunk, float factor)
{
  const int  index = z * 9 + x;
  float diff = _vertices[index].position.y - chunk->mVertices[z * 17 + x].y;
  _vertices[z * 9 + x].depth = diff < 0.0f ? 0.0f : (std::min(1.0f, std::max(0.0f, (diff + 1.0f) * factor)));
}

int liquid_layer::get_lod_level(glm::vec3 const& camera_pos) const
{
  glm::vec3 const& center_vertex (_vertices[5 * 9 + 4].position);
  // this doesn't look like it's using the right length function...
  // auto const dist ((center_vertex - camera_pos).length());
  float const dist = misc::dist(center_vertex, camera_pos);

  return dist < 1000.f ? 0
       : dist < 2000.f ? 1
       : dist < 4000.f ? 2
       : 3;
}
// if ocean and all subchunks are at max depth
bool liquid_layer::check_fatigue() const
{
    // only oceans have fatigue
    if (_liquid_type != liquid_basic_types_ocean)
    {
        return false;
    }

    for (int z = 0; z < 8; ++z)
    {
        for (int x = 0; x < 8; ++x)
        {
            if (!(hasSubchunk(x, z) && subchunk_at_max_depth(x, z)))
            {
                return false;
            }
        }
    }

    return true;
}

void liquid_layer::update_attributes(MH2O_Attributes& attributes)
{
    if (check_fatigue())
    {
        attributes.fishable = 0xFFFFFFFFFFFFFFFF;
        attributes.fatigue = 0xFFFFFFFFFFFFFFFF;

        _fatigue_enabled = true;
    }
    else
    {
        _fatigue_enabled = false;
        for (int z = 0; z < 8; ++z)
        {
            for (int x = 0; x < 8; ++x)
            {
                if (hasSubchunk(x, z))
                {
                    // todo : find out when fishable isn't set. maybe lava/slime or very shallow water ?
                    // Most likely when subchunk is entirely above terrain.
                    misc::set_bit(attributes.fishable, x, z, true);

                    // only oceans have fatigue
                    // warning: not used by TrinityCore
                    if (_liquid_type == liquid_basic_types_ocean && subchunk_at_max_depth(x, z))
                    {
                        misc::set_bit(attributes.fatigue, x, z, true);
                    }
                }
            }
        }
    }
}

bool liquid_layer::subchunk_at_max_depth(int x, int z) const
{
    for (int id_z = z; id_z <= z + 1; ++id_z)
    {
        for (int id_x = x; id_x <= x + 1; ++id_x)
        {
            if (_vertices[id_x + 9 * id_z].depth < 1.f)
            {
                return false;
            }
        }
    }

    return true;
}

