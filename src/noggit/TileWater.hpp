// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/ChunkWater.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/tool_enums.hpp>
#include <opengl/context.hpp>
#include <opengl/types.hpp>
#include <noggit/rendering/LiquidTextureManager.hpp>
#include <noggit/rendering/LiquidRender.hpp>
#include <util/sExtendableArray.hpp>

#include <memory>
#include <vector>
#include <array>
#include <set>

class MapTile;
class liquid_layer;
enum LiquidLayerUpdateFlags;

namespace BlizzardArchive
{
  class ClientFile;
}

namespace Noggit::Rendering
{
  class LiquidRender;
}


class TileWater
{
  friend class Noggit::Rendering::LiquidRender;

public:
  TileWater(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava);

  ChunkWater* getChunk(int x, int z);

  void readFromFile(BlizzardArchive::ClientFile& theFile, size_t basePos);
  void saveToFile(util::sExtendableArray& lADTFile, int& lMHDR_Position, int& lCurrentPosition);

  void draw ( math::frustum const& frustum
            , const glm::vec3& camera
            , bool camera_moved
            , OpenGL::Scoped::use_program& water_shader
            , int animtime
            , int layer
            , display_mode display
            , Noggit::Rendering::LiquidTextureManager* tex_manager
            );
  bool hasData(size_t layer);
  bool hasData() { return _has_data; };

  void CropMiniChunk(int x, int z, MapChunk* chunkTerrain);

  void autoGen(float factor);

  void update_underground_vertices_depth();

  void setType(int type, size_t layer);
  int getType(size_t layer);

  std::array<glm::vec3, 2>& getExtents() { return _extents; };

  [[nodiscard]]
  bool isVisible(const math::frustum& frustum) const;

  void setWatermapImage(QImage const& baseimage, float min_height, float max_height, int mode, bool tiledEdges);

  void tagExtents(bool state) { _extents_changed = state; };
  void tagUpdate() { _renderer.tagUpdate(); };

  Noggit::Rendering::LiquidRender* renderer() { return &_renderer; };

  [[nodiscard]]
  bool needsUpdate() { return _renderer.needsUpdate() || _extents_changed; };

  void recalcExtents();

private:

  MapTile *tile;
  Noggit::Rendering::LiquidRender _renderer;

  std::unique_ptr<ChunkWater> chunks[16][16];
  std::array<glm::vec3, 2> _extents;

  bool _has_data = true;
  bool _extents_changed = true;

  float xbase;
  float zbase;
};
