// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <math/frustum.hpp>
#include <noggit/liquid_layer.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/tool_enums.hpp>
#include <util/sExtendableArray.hpp>

#include <vector>
#include <set>
#include <optional>

class MapChunk;
class TileWater;

namespace BlizzardArchive
{
  class ClientFile;
}

class ChunkWater
{
public:
  ChunkWater() = delete;
  explicit ChunkWater(MapChunk* chunk, TileWater* water_tile, float x, float z, bool use_mclq_green_lava);

  ChunkWater (ChunkWater const&) = delete;
  ChunkWater (ChunkWater&&) = delete;
  ChunkWater& operator= (ChunkWater const&) = delete;
  ChunkWater& operator= (ChunkWater&&) = delete;

  void from_mclq(std::vector<mclq>& layers);
  void fromFile(BlizzardArchive::ClientFile& f, size_t basePos);
  void save(util::sExtendableArray& adt, int base_pos, int& header_pos, int& current_pos);
  void save_mclq(util::sExtendableArray& adt, int mcnk_pos, int& current_pos);

  bool is_visible ( const float& cull_distance
                  , const math::frustum& frustum
                  , const glm::vec3& camera
                  , display_mode display
                  ) const;

  void autoGen(MapChunk* chunk, float factor);
  void update_underground_vertices_depth(MapChunk* chunk);
  void CropWater(MapChunk* chunkTerrain);

  void setType(int type, size_t layer);
  int getType(size_t layer) const;
  bool hasData(size_t layer) const;
  void tagUpdate();

  std::vector<liquid_layer>* getLayers() { return &_layers; };

  // update every layer's render
  void update_layers();
  float getMinHeight() { return vmin.y; };
  float getMaxHeight() { return vmax.y; }

  void paintLiquid( glm::vec3 const& pos
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
                  );

  MapChunk* getChunk() { return _chunk; };
  TileWater* getWaterTile() { return _water_tile; };

  MH2O_Attributes& const getAttributes() { return attributes; };

  float xbase, zbase;

  int layer_count() const { return _layers.size(); }

private:
  MH2O_Attributes attributes;

  glm::vec3 vmin, vmax, vcenter;
  bool _use_mclq_green_lava;

  // remove empty layers
  void cleanup();

  void copy_height_to_layer(liquid_layer& target, glm::vec3 const& pos, float radius);

  bool _auto_update_attributes = true;
  // updates attributes for all layers
  void update_attributes();

  std::vector<liquid_layer> _layers;
  int _layer_count = 0;

  MapChunk* _chunk;
  TileWater* _water_tile;

  friend class MapView;
};
