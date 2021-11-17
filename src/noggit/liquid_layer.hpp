// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/MPQ.h>
#include <opengl/scoped.hpp>
#include <array>
#include <glm/vec2.hpp>

class MapChunk;
class sExtendableArray;
class ChunkWater;

enum LiquidLayerUpdateFlags
{
  ll_HEIGHT = 0x1,
  ll_DEPTH = 0x2,
  ll_UV = 0x4,
  ll_TYPE = 0x8,
  ll_FLAGS = 0x10
};

// handle liquids like oceans, lakes, rivers, slime, magma
class liquid_layer
{

public:
  liquid_layer() = delete;
  liquid_layer(ChunkWater* chunk, glm::vec3 const& base, float height, int liquid_id);
  liquid_layer(ChunkWater* chunk, glm::vec3 const& base, mclq& liquid, int liquid_id);
  liquid_layer(ChunkWater* chunk, MPQFile &f, std::size_t base_pos, glm::vec3 const& base, MH2O_Information const& info, std::uint64_t infomask);

  liquid_layer(liquid_layer const& other);
  liquid_layer(liquid_layer&&);

  liquid_layer& operator=(liquid_layer&&);
  liquid_layer& operator=(liquid_layer const& other);

  void save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const;

  void changeLiquidID(int id);

  void crop(MapChunk* chunk);
  void update_opacity(MapChunk* chunk, float factor);

  std::array<glm::vec3, 9 * 9>& getVertices() { return _vertices; };
  std::array<float, 9 * 9>& getDepth() { return _depth; };
  std::array<glm::vec2, 9 * 9>& getTexCoords() { return _tex_coords; };

  float min() const { return _minimum; }
  float max() const { return _maximum; }
  int liquidID() const { return _liquid_id; }

  bool hasSubchunk(int x, int z, int size = 1) const;
  void setSubchunk(int x, int z, bool water);

  std::uint64_t getSubchunks() { return _subchunks; };

  bool empty() const { return !_subchunks; }
  bool full() const { return _subchunks == std::uint64_t(-1); }
  void clear() { _subchunks = std::uint64_t(0); }

  void paintLiquid(glm::vec3 const& pos
                  , float radius
                  , bool add
                  , math::radians const& angle
                  , math::radians const& orientation
                  , bool lock
                  , glm::vec3 const& origin
                  , bool override_height
                  , MapChunk* chunk
                  , float opacity_factor
                  );

  void copy_subchunk_height(int x, int z, liquid_layer const& from);

  ChunkWater* getChunk() { return _chunk; };

private:
  void update_min_max();
  void update_vertex_opacity(int x, int z, MapChunk* chunk, float factor);
  int get_lod_level(glm::vec3 const& camera_pos) const;
  void set_lod_level(int lod_level);

  int _liquid_id;
  int _liquid_vertex_format;
  float _minimum;
  float _maximum;

  std::uint64_t _subchunks;
  std::array<glm::vec3, 9 * 9> _vertices;
  std::array<float, 9 * 9> _depth;
  std::array<glm::vec2, 9 * 9> _tex_coords;

  std::map<int, std::vector<std::uint16_t>> _indices_by_lod;


private:
  glm::vec3 pos;
  ChunkWater* _chunk;
};
