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
  , _liquid_vertex_format(0)
  , _minimum(height)
  , _maximum(height)
  , _subchunks(0)
  , pos(base)
  , _chunk(chunk)
{
  if (!gLiquidTypeDB.CheckIfIdExists(_liquid_id))
    _liquid_id = 1;

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
       unsigned v_index = z * 9 + x;
      _tex_coords[v_index] = default_uv(x, z);
      _depth[v_index] = 1.0f;
      _vertices[v_index] = glm::vec3(
                            pos.x + UNITSIZE * x
                            , height
                            , pos.z + UNITSIZE * z
                            );
    }
  }

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
    _liquid_id = 1;

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

      if (_liquid_vertex_format == 1)
      {
        _depth[v_index] = 1.0f;
        _tex_coords[v_index] = glm::vec2(static_cast<float>(v.magma.x) / 255.f, static_cast<float>(v.magma.y) / 255.f);
      }
      else
      {
        _depth[v_index] = static_cast<float>(v.water.depth) / 255.f;
        _tex_coords[v_index] = default_uv(x, z);
      }

      _vertices[v_index] = glm::vec3(
                            pos.x + UNITSIZE * x
                            // sometimes there's garbage data on unused tiles that mess things up
                            , std::max(std::min(v.height, _maximum), _minimum)
                            , pos.z + UNITSIZE * z
                            );
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
    _liquid_id = 1;

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
  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      const unsigned v_index = z * 9 + x;
      _tex_coords[v_index] = default_uv(x, z);
      _depth[v_index] = 1.0f;
      _vertices[v_index] = glm::vec3(
          pos.x + UNITSIZE * x
        , _minimum
        , pos.z + UNITSIZE * z
      );
    }
  }

  if (info.ofsHeightMap)
  {
    f.seek(base_pos + info.ofsHeightMap);

    if (_liquid_vertex_format == 0 || _liquid_vertex_format == 1)
    {

      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          f.read(&_vertices[z * 9 + x].y, sizeof(float));
        }
      }
    }

    if (_liquid_vertex_format == 1)
    {
      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          mh2o_uv uv;
          f.read(&uv, sizeof(mh2o_uv));
          _tex_coords[z * 9 + x] = 
            { static_cast<float>(uv.x) / 255.f
            , static_cast<float>(uv.y) / 255.f
            };
        }
      }
    }

    if (_liquid_vertex_format == 0 || _liquid_vertex_format == 2)
    {
      for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
      {
        for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
        {
          std::uint8_t depth;
          f.read(&depth, sizeof(std::uint8_t));
          _depth[z * 9 + x] = static_cast<float>(depth) / 255.f;
        }
      }
    }
  }

  
}

liquid_layer::liquid_layer(liquid_layer&& other)
  : _liquid_id(other._liquid_id)
  , _liquid_vertex_format(other._liquid_vertex_format)
  , _minimum(other._minimum)
  , _maximum(other._maximum)
  , _subchunks(other._subchunks)
  , _vertices(other._vertices)
  , _depth(other._depth)
  , _tex_coords(other._tex_coords)
  , _indices_by_lod(other._indices_by_lod)
  , pos(other.pos)
  , _chunk(other._chunk)
{
  changeLiquidID(_liquid_id);
}

liquid_layer::liquid_layer(liquid_layer const& other)
  : _liquid_id(other._liquid_id)
  , _liquid_vertex_format(other._liquid_vertex_format)
  , _minimum(other._minimum)
  , _maximum(other._maximum)
  , _subchunks(other._subchunks)
  , _vertices(other._vertices)
  , _depth(other._depth)
  , _tex_coords(other._tex_coords)
  , _indices_by_lod(other._indices_by_lod)
  , pos(other.pos)
  , _chunk(other._chunk)
{
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
  std::swap(_depth, other._depth);
  std::swap(_tex_coords, other._tex_coords);
  std::swap(pos, other.pos);
  std::swap(_indices_by_lod, other._indices_by_lod);
  std::swap(_chunk, other._chunk);

  changeLiquidID(_liquid_id);
  other.changeLiquidID(other._liquid_id);

  return *this;
}

liquid_layer& liquid_layer::operator=(liquid_layer const& other)
{
  changeLiquidID(other._liquid_id);
  _liquid_vertex_format = other._liquid_vertex_format;
  _minimum = other._minimum;
  _maximum = other._maximum;
  _subchunks = other._subchunks;
  _vertices = other._vertices;
  _depth = other._depth;
  _tex_coords = other._tex_coords;
  pos = other.pos;
  _indices_by_lod = other._indices_by_lod;
  _chunk = other._chunk;

  return *this;
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

  info.ofsHeightMap = current_pos - base_pos;

  int vertices_count = (info.width + 1) * (info.height + 1);

  if (_liquid_vertex_format == 0 || _liquid_vertex_format == 1)
  {
    adt.Extend(vertices_count * sizeof(float));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        memcpy(adt.GetPointer<char>(current_pos), &_vertices[z * 9 + x].y, sizeof(float));
        current_pos += sizeof(float);
      }
    }
  }

  if (_liquid_vertex_format == 1)
  {
    adt.Extend(vertices_count * sizeof(mh2o_uv));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        mh2o_uv uv;
        uv.x = static_cast<std::uint16_t>(std::min(_tex_coords[z * 9 + x].x * 255.f, 65535.f));
        uv.y = static_cast<std::uint16_t>(std::min(_tex_coords[z * 9 + x].y * 255.f, 65535.f));

        memcpy(adt.GetPointer<char>(current_pos), &uv, sizeof(mh2o_uv));
        current_pos += sizeof(mh2o_uv);
      }
    }
  }

  if (_liquid_vertex_format == 0 || _liquid_vertex_format == 2)
  {
    adt.Extend(vertices_count * sizeof(std::uint8_t));

    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        std::uint8_t depth = static_cast<std::uint8_t>(std::min(_depth[z * 9 + x] * 255.0f, 255.f));
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

    switch (lLiquidTypeRow.getInt(LiquidTypeDB::Type))
    {
    case 2: // magma
    case 3: // slime
      _liquid_vertex_format = 1;
      break;
    default:
      _liquid_vertex_format = 0;
      break;
    }
  }
  catch (...)
  {
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

          if (_vertices[water_index].y < chunk->mVertices[terrain_index].y
            && _vertices[water_index + 1].y < chunk->mVertices[terrain_index + 1].y
            && _vertices[water_index + 9].y < chunk->mVertices[terrain_index + 17].y
            && _vertices[water_index + 10].y < chunk->mVertices[terrain_index + 18].y
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
      if (misc::getShortestDist(cursor_pos, _vertices[id], UNITSIZE) <= radius)
      {
        if (add)
        {
          for (int index : {id, id + 1, id + 9, id + 10})
          {
            bool no_subchunk = !hasSubchunk(x, z);
            bool in_range = misc::dist(cursor_pos, _vertices[index]) <= radius;

            if (no_subchunk || (in_range && override_height))
            {
              _vertices[index].y = misc::angledHeight(ref, _vertices[index], angle, orientation);
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

  for (glm::vec3& v : _vertices)
  {
    if (hasSubchunk(std::min(x, 7), std::min(z, 7)))
    {
      _maximum = std::max(_maximum, v.y);
      _minimum = std::min(_minimum, v.y);
    }

    if (++x == 9)
    {
      z++;
      x = 0;
    }
  }

  // lvf = 2 means the liquid height is 0, switch to lvf 0 when that's not the case
  if (_liquid_vertex_format == 2 && (!misc::float_equals(0.f, _minimum) || !misc::float_equals(0.f, _maximum)))
  {
    _liquid_vertex_format = 0;
  }
  // use lvf 2 when possible to save space
  else if (_liquid_vertex_format == 0 && misc::float_equals(0.f, _minimum) && misc::float_equals(0.f, _maximum))
  {
    _liquid_vertex_format = 2;
  }
}

void liquid_layer::copy_subchunk_height(int x, int z, liquid_layer const& from)
{
  int id = 9 * z + x;

  for (int index : {id, id + 1, id + 9, id + 10})
  {
    _vertices[index].y = from._vertices[index].y;
  }

  setSubchunk(x, z, true);
}

void liquid_layer::update_vertex_opacity(int x, int z, MapChunk* chunk, float factor)
{
  float diff = _vertices[z * 9 + x].y - chunk->mVertices[z * 17 + x].y;
  _depth[z * 9 + x] = diff < 0.0f ? 0.0f : (std::min(1.0f, std::max(0.0f, (diff + 1.0f) * factor)));
}

int liquid_layer::get_lod_level(glm::vec3 const& camera_pos) const
{
  auto const& center_vertex (_vertices[5 * 9 + 4]);
  auto const dist ((center_vertex - camera_pos).length());

  return dist < 1000.f ? 0
       : dist < 2000.f ? 1
       : dist < 4000.f ? 2
       : 3;
}

