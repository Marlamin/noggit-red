// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "map_horizon.h"

#include <noggit/Log.h>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/map_index.hpp>
#include <noggit/MapTile.h>
#include <noggit/World.h>
#include <opengl/context.hpp>
#include <opengl/context.inl>

#include <sstream>
#include <bitset>

struct color
{
  color(unsigned char r, unsigned char g, unsigned char b)
    : _r(r)
    , _g(g)
    , _b(b)
  {}

  uint32_t to_int() const {
    return (_b) | (_g << 8) | (_r << 16) | (uint32_t)(0xFFu << 24);
  }

  operator uint32_t () const {
    return to_int();
  }

  unsigned char _r;
  unsigned char _g;
  unsigned char _b;
};

struct ranged_color
{
  ranged_color (const color& c, const int16_t& start, const int16_t& stop)
    : _color (c)
    , _start (start)
    , _stop (stop)
  {}

  const color   _color;
  const int16_t _start;
  const int16_t _stop;
};

static inline color lerp_color(const color& start, const color& end, float t)
{
  return color ( (end._r) * t + (start._r) * (1.0 - t)
               , (end._g) * t + (start._g) * (1.0 - t)
               , (end._b) * t + (start._b) * (1.0 - t)
               );
}

static inline uint32_t color_for_height (int16_t height)
{
  static const ranged_color colors[] =
    { ranged_color (color (20, 149, 7), 0, 600)
    , ranged_color (color (137, 84, 21), 600, 1200)
    , ranged_color (color (96, 96, 96), 1200, 1600)
    , ranged_color (color (255, 255, 255), 1600, 0x7FFF)
    };
  static const size_t num_colors (sizeof (colors) / sizeof (ranged_color));

  if (height < colors[0]._start)
  {
    return color (0, 0, 255 + std::max (height / 2.0, -255.0));
  }
  else if (height >= colors[num_colors - 1]._stop)
  {
    return colors[num_colors]._color;
  }

  float t (1.0);
  size_t correct_color (num_colors - 1);

  for (size_t i (0); i < num_colors - 1; ++i)
  {
    if (height >= colors[i]._start && height < colors[i]._stop)
    {
      t = float(height - colors[i]._start) / float (colors[i]._stop - colors[i]._start);
      correct_color = i;
      break;
    }
  }

  return lerp_color(colors[correct_color]._color, colors[correct_color + 1]._color, t);
}
namespace Noggit
{

map_horizon::map_horizon(const std::string& basename, const MapIndex * const index)
{
  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdl";
  _filename = filename.str();

  if (!Application::NoggitApplication::instance()->clientData()->exists(_filename))
  {
    LogError << "file \"World\\Maps\\" << basename << "\\" << basename << ".wdl\" does not exist." << std::endl;
    return;
  }

  BlizzardArchive::ClientFile wdl_file (_filename, Application::NoggitApplication::instance()->clientData());

  uint32_t fourcc;
  uint32_t size;

  bool done = false;

  do
  {
    wdl_file.read(&fourcc, 4);
    wdl_file.read(&size, 4);

    switch (fourcc)
    {
      case 'MVER':
      {
        uint32_t version;
        wdl_file.read(&version, 4);
        assert(size == 4 && version == 18);

        break;
      }
      // todo: handle those too ?
      case 'MWMO':
      {
        {
            char const* lCurPos = reinterpret_cast<char const*>(wdl_file.getPointer());
            char const* lEnd = lCurPos + size;
        
            while (lCurPos < lEnd)
            {
                mWMOFilenames.push_back(BlizzardArchive::ClientData::normalizeFilenameInternal(std::string(lCurPos)));
                lCurPos += strlen(lCurPos) + 1;
            }
        }
        wdl_file.seekRelative(size);
        break;
      }
      case 'MWID':
          wdl_file.seekRelative(size);
          break;
          // TODO
      case 'MODF':
      {
        wdl_file.seekRelative(size);
        break;
        // {
        //     ENTRY_MODF const* modf_ptr = reinterpret_cast<ENTRY_MODF const*>(wdl_file.getPointer());
        //     for (unsigned int i = 0; i < size / sizeof(ENTRY_MODF); ++i)
        //     {
        //         lWMOInstances.push_back(modf_ptr[i]);
        //     }
        // }
        // break;
      }
      case 'MAOF':
      {
        assert(size == 64 * 64 * sizeof(uint32_t));

        uint32_t mare_offsets[64][64];
        wdl_file.read(mare_offsets, 64 * 64 * sizeof(uint32_t));

        // - MARE and MAHO by offset ---------------------------
        for (size_t y(0); y < 64; ++y)
        {
          for (size_t x(0); x < 64; ++x)
          {
            if (!mare_offsets[y][x])
            {
              continue;
            }

            wdl_file.seek(mare_offsets[y][x]);
            wdl_file.read(&fourcc, 4);
            wdl_file.read(&size, 4);

            assert(fourcc == 'MARE');
            assert(size == 0x442);

            _tiles[y][x] = std::make_unique<map_horizon_tile>();

            //! \todo There also is MAHO giving holes into this heightmap.
            wdl_file.read(_tiles[y][x]->height_17, 17 * 17 * sizeof(int16_t));
            wdl_file.read(_tiles[y][x]->height_16, 16 * 16 * sizeof(int16_t));

            if (wdl_file.getPos() < wdl_file.getSize())
            {
                wdl_file.read(&fourcc, 4);
                if (fourcc == 'MAHO')
                {
                    wdl_file.read(&size, 4);
                    assert(size == 0x20);
                    wdl_file.read(_tiles[y][x]->holes, 16 * sizeof(int16_t));
                }
            }

          }
        }
        done = true;
        break;
      }
      default:
        LogError << "unknown chunk in wdl: code=" << fourcc << std::endl;
        wdl_file.seekRelative(size);
        break;
    }
  } while (!done && !wdl_file.isEof());

  wdl_file.close();

  set_minimap(index);
}

void map_horizon::set_minimap(const MapIndex* const index)
{
    _qt_minimap = QImage(16 * 64, 16 * 64, QImage::Format_ARGB32);
    _qt_minimap.fill(Qt::transparent);

    for (int y(0); y < 64; ++y)
    {
        for (int x(0); x < 64; ++x)
        {
            if (_tiles[y][x])
            {
                //! \todo There also is a second heightmap appended which has additional 16*16 pixels.
                //! \todo There also is MAHO giving holes into this heightmap.

                for (int j(0); j < 16; ++j)
                {
                    for (int i(0); i < 16; ++i)
                    {
                        //! \todo R and B are inverted here
                        _qt_minimap.setPixel(x * 16 + i, y * 16 + j, color_for_height(_tiles[y][x]->height_17[j][i]));
                    }
                }
            }
            // the adt exist but there's no data in the wdl
            else if (index->hasTile(TileIndex(x, y)))
            {
                for (int j(0); j < 16; ++j)
                {
                    for (int i(0); i < 16; ++i)
                    {
                        _qt_minimap.setPixel(x * 16 + i, y * 16 + j, color(200, 100, 25));
                    }
                }
            }
        }
    }
}

Noggit::map_horizon_tile* map_horizon::get_horizon_tile(int y, int x)
{
    return _tiles[y][x].get();
}

int16_t map_horizon::getWdlheight(MapTile* tile, float x, float y)
{
    int cx = std::min(std::max(static_cast<int>(x / CHUNKSIZE), 0), 15);
    int cy = std::min(std::max(static_cast<int>(y / CHUNKSIZE), 0), 15);

    x -= cx * CHUNKSIZE;
    y -= cy * CHUNKSIZE;

    int row = static_cast<int>(y / (UNITSIZE * 0.5f) + 0.5f);
    int col = static_cast<int>((x - UNITSIZE * 0.5f * (row % 2)) / UNITSIZE + 0.5f);
    bool inner = (row % 2) == 1;

    if (row < 0 || col < 0 || row > 16 || col >(inner ? 8 : 9))
        return 0;

    // truncate and clamp the float value
    auto chunk = tile->getChunk(cx, cy);
    // float height = heights[cy * 16 + cx][17 * (row / 2) + (inner ? 9 : 0) + col];
    float height = chunk->getHeightmap()[17 * (row / 2) + (inner ? 9 : 0) + col].y;
    return std::min(std::max(static_cast<int16_t>(height), static_cast<int16_t>(SHRT_MIN)), static_cast<int16_t>(SHRT_MAX));
}

void map_horizon::update_horizon_tile(MapTile* mTile)
{
    auto tile_index = mTile->index;

    // calculate the heightmap as a short array
    float x, y;
    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 17; j++)
        {
            // outer - correct
            x = j * CHUNKSIZE;
            y = i * CHUNKSIZE;

            if (!_tiles[tile_index.z][tile_index.x].get()) // tile has not been initialised
                //     continue;
            {
                _tiles[tile_index.z][tile_index.x] = std::make_unique<map_horizon_tile>();
                // do we need to use memcpy as well ?
            }
            // only works for initialised
            _tiles[tile_index.z][tile_index.x].get()->height_17[i][j] = getWdlheight(mTile, x, y);

            // inner - close enough; correct values appear to use some form of averaging
            if (i < 16 && j < 16)
                _tiles[tile_index.z][tile_index.x].get()->height_16[i][j] = getWdlheight(mTile, x + CHUNKSIZE / 2.0f, y + CHUNKSIZE / 2.0f);
        }
    }
    // Holes
    for (int i = 0; i < 16; ++i)
    {
        std::bitset<16>wdlHoleMask(0);

        for (int j = 0; j < 16; ++j)
        {
            auto chunk = mTile->getChunk(j, i);
            // the ordering seems to be : short array = Y axis, flags values = X axis and the values are for a whole chunk.

            std::bitset<16> holeBits(chunk->getHoleMask());

            if (holeBits.count() == 16) // if all holes are set in a chunk
                wdlHoleMask.set(j, true);
        }
        _tiles[tile_index.z][tile_index.x].get()->holes[i] = static_cast<int16_t>(wdlHoleMask.to_ulong());
    }
}

void map_horizon::save_wdl(World* world, bool regenerate)
{
    world->wait_for_all_tile_updates();

    std::stringstream filename;
    filename << "World\\Maps\\" << world->basename << "\\" << world->basename << ".wdl";
    //Log << "Saving WDL \"" << filename << "\"." << std::endl;

    sExtendableArray wdlFile = sExtendableArray();
    int curPos = 0;

    // MVER
    //  {
    wdlFile.Extend(8 + 0x4);
    SetChunkHeader(wdlFile, curPos, 'MVER', 4);

    // MVER data
    *(wdlFile.GetPointer<int>(8)) = 18; // write version 18
    curPos += 8 + 0x4;
    //  }

    // MWMO
    //  {
    wdlFile.Extend(8);
    SetChunkHeader(wdlFile, curPos, 'MWMO', 0);
    curPos += 8;
    //  }

    // MWID
    //  {
    wdlFile.Extend(8);
    SetChunkHeader(wdlFile, curPos, 'MWID', 0);
    curPos += 8;
    //  }

    // TODO : MODF
    //  {
    wdlFile.Extend(8);
    SetChunkHeader(wdlFile, curPos, 'MODF', 0);
    curPos += 8;
    //  }

    //uint32_t mare_offsets[64][64] = { 0 };
    // MAOF
    //  {
    wdlFile.Extend(8);
    SetChunkHeader(wdlFile, curPos, 'MAOF', 64 * 64 * 4);
    curPos += 8;
    wdlFile.Extend(64 * 64 * 4);
    uint mareoffset = curPos + 64 * 64 * 4;

    for (int y = 0; y < 64; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            TileIndex index(x, y);

            bool has_tile = world->mapIndex.hasTile(index);
            // write offset in MAOF entry
            *(wdlFile.GetPointer<uint>(curPos)) = has_tile ? mareoffset : 0;

            if (has_tile)
            {
                // MARE Header
                //  {
                wdlFile.Extend(8);
                SetChunkHeader(wdlFile, mareoffset, 'MARE', (2 * (17 * 17)) + (2 * (16 * 16))); // outer heights+inner heights
                mareoffset += 8;

                // this might be invalid if map had no WDL
                Noggit::map_horizon_tile* horizon_tile = get_horizon_tile(y, x);

                // laod tile and extract WDL data
                if (!horizon_tile || regenerate)
                {
                    bool unload = !world->mapIndex.tileLoaded(index) && !world->mapIndex.tileAwaitingLoading(index);
                    MapTile* mTile = world->mapIndex.loadTile(index);

                    if (mTile)
                        mTile->wait_until_loaded();

                    update_horizon_tile(mTile);
                    if (unload)
                        world->mapIndex.unloadTile(index);

                    auto test = get_horizon_tile(y, x);
                    horizon_tile = get_horizon_tile(y, x);
                }
                if (!horizon_tile)
                {
                    return; // failed to generate data somehow
                    LogError << "Failed to generate the WDL file." << std::endl;
                }

                wdlFile.Insert(mareoffset, sizeof(Noggit::map_horizon_tile::height_17), reinterpret_cast<char*>(&horizon_tile->height_17));
                mareoffset += sizeof(Noggit::map_horizon_tile::height_17);
                wdlFile.Insert(mareoffset, sizeof(Noggit::map_horizon_tile::height_16), reinterpret_cast<char*>(&horizon_tile->height_16));
                mareoffset += sizeof(Noggit::map_horizon_tile::height_16);

                // MAHO (maparea holes) MAHO was added in WOTLK ?
                //  {
                wdlFile.Extend(8);
                SetChunkHeader(wdlFile, mareoffset, 'MAHO', (2 * 16)); // 1 hole mask for each chunk
                mareoffset += 8;
                wdlFile.Extend(32);
                for (int i = 0; i < 16; ++i)
                {
                    wdlFile.Insert(mareoffset, 2, (char*)&horizon_tile->holes[i]);
                    mareoffset += 2;
                }
            }
            curPos += 4;
        }
    }
    BlizzardArchive::ClientFile f(filename.str(), Noggit::Application::NoggitApplication::instance()->clientData(),
    BlizzardArchive::ClientFile::NEW_FILE);
    f.setBuffer(wdlFile.data);
    f.save();
    f.close();

    set_minimap(&world->mapIndex);
}

map_horizon::minimap::minimap(const map_horizon& horizon)
{
  std::vector<uint32_t> texture(1024 * 1024);

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (!horizon._tiles[y][x])
        continue;

      //! \todo There also is a second heightmap appended which has additional 16*16 pixels.

      // use the (nearly) full resolution available to us.
      // the data is layed out as a triangle fans with with 17 outer values
      // and 16 midpoints per tile. which in turn means:
      //      _tiles[y][x]->height_17[16][16] == _tiles[y][x + 1]->height_17[0][0]
      for (size_t j (0); j < 16; ++j)
      {
        for (size_t i (0); i < 16; ++i)
        {
          texture[(y * 16 + j) * 1024 + x * 16 + i] = color_for_height (horizon._tiles[y][x]->height_17[j][i]);
        }
      }
    }
  }

  bind();
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture.data());
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

map_horizon::render::render(const map_horizon& horizon)
{
  std::vector<glm::vec3> vertices;

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (!horizon._tiles[y][x])
        continue;

      _batches[y][x] = map_horizon_batch (static_cast<std::uint32_t>(vertices.size()), 17 * 17 + 16 * 16);

      for (size_t j (0); j < 17; ++j)
      {
        for (size_t i (0); i < 17; ++i)
        {
          vertices.emplace_back ( TILESIZE * (x + i / 16.0f)
                                , horizon._tiles[y][x]->height_17[j][i]
                                , TILESIZE * (y + j / 16.0f)
                                );
        }
      }

      for (size_t j (0); j < 16; ++j)
      {
        for (size_t i (0); i < 16; ++i)
        {
          vertices.emplace_back ( TILESIZE * (x + (i + 0.5f) / 16.0f)
                                , horizon._tiles[y][x]->height_16[j][i]
                                , TILESIZE * (y + (j + 0.5f) / 16.0f)
                                );
        }
      }
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER, glm::vec3> (_vertex_buffer, vertices, GL_STATIC_DRAW);
}

static inline uint32_t outer_index(const map_horizon_batch &batch, int y, int x)
{
  return batch.vertex_start + y * 17 + x;
};

static inline uint32_t inner_index(const map_horizon_batch &batch, int y, int x)
{
  return batch.vertex_start + 17 * 17 + y * 16 + x;
};

void map_horizon::render::draw( glm::mat4x4 const& model_view
                              , glm::mat4x4 const& projection
                              , MapIndex *index
                              , const glm::vec3& color
                              , const float& cull_distance
                              , const math::frustum& frustum
                              , const glm::vec3& camera 
                              , display_mode display
                              )
{
  std::vector<uint32_t> indices;

  const TileIndex current_index(camera);
  const int lrr = 2;

  for (size_t y (current_index.z - lrr); y <= current_index.z + lrr; ++y)
  {
    for (size_t x (current_index.x - lrr); x < current_index.x + lrr; ++x)
    {
      // x and y are unsigned so negative signed int value are positive and > 63
      if (x > 63 || y > 63)
      {
        continue;
      }

      map_horizon_batch const& batch = _batches[y][x];

      if (batch.vertex_count == 0)
        continue;

      for (int j (0); j < 16; ++j)
      {
        for (int i (0); i < 16; ++i)
        {
          // do not draw over visible chunks

          /* TODO: when this optimization is turned off, we end up with inconsistent rendering between chunks and horizon batches.
           * Potentially it is caused by inconsistent coordinate space in visibility checking or chunk update system.
          if (index->tileLoaded({y, x}) && index->getTile({y, x})->getChunk(j, i)->is_visible(cull_distance, frustum, camera, display))
          {
            //continue;
          }
          */

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j, i));
          indices.push_back (outer_index (batch, j + 1, i));

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j + 1, i));
          indices.push_back (outer_index (batch, j + 1, i + 1));

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j + 1, i + 1));
          indices.push_back (outer_index (batch, j, i + 1));

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j, i + 1));
          indices.push_back (outer_index (batch, j, i));
        }
      }
    }
  }

  if (_map_horizon_program)
  {
    gl.bufferSubData<GL_ELEMENT_ARRAY_BUFFER, std::uint32_t>(_index_buffer, 0, indices);
  }
  else
  {
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint32_t>(_index_buffer, indices, GL_DYNAMIC_DRAW);

    _map_horizon_program.reset
      ( new OpenGL::program
          { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("horizon_vs") }
          , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("horizon_fs") }
          }
      );
  
    _vaos.upload();
  }
   

  OpenGL::Scoped::use_program shader {*_map_horizon_program.get()};

  OpenGL::Scoped::vao_binder const _ (_vao);

  shader.uniform ("model_view", model_view);
  shader.uniform ("projection", projection);
  shader.uniform ("color", glm::vec3(color.x, color.y, color.z));

  shader.attrib ("position", _vertex_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);

  OpenGL::Scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> indices_binder (_index_buffer);

  
  gl.drawElements (GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

}
