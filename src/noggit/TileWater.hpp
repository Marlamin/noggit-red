// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <noggit/ChunkWater.hpp>
#include <noggit/MPQ.h>
#include <noggit/MapHeaders.h>
#include <noggit/tool_enums.hpp>
#include <opengl/context.hpp>
#include <opengl/types.hpp>
#include <noggit/LiquidTextureManager.hpp>

#include <memory>
#include <vector>
#include <array>
#include <set>

class MapTile;
class liquid_layer;
class sExtendableArray;
enum LiquidLayerUpdateFlags;

class TileWater
{
  struct LiquidTexBinding
  {
    unsigned tex_array;
    unsigned type;
    unsigned pad0;
    unsigned pad1;
  };

public:
  TileWater(MapTile *pTile, float pXbase, float pZbase, bool use_mclq_green_lava);

  ChunkWater* getChunk(int x, int z);

  void readFromFile(MPQFile &theFile, size_t basePos);
  void saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);

  void draw ( math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool camera_moved
            , opengl::scoped::use_program& water_shader
            , int animtime
            , int layer
            , display_mode display
            , LiquidTextureManager* tex_manager
            );
  bool hasData(size_t layer);
  void CropMiniChunk(int x, int z, MapChunk* chunkTerrain);

  void autoGen(float factor);

  void setType(int type, size_t layer);
  int getType(size_t layer);

  void registerChunkLayer(liquid_layer* layer);
  void unregisterChunkLayer(liquid_layer* layer);
  void registerChunkLayerUpdate(unsigned flags);
  void doneChunkLayerUpdate() { _update_flags = 0; };
  void markBuffersDirty() { _need_buffer_update = true; };
  void updateLayerData(LiquidTextureManager* tex_manager);

  void prepareBufferData(std::size_t layer_index);

  void unload();

  void upload();

private:

  MapTile *tile;
  std::unique_ptr<ChunkWater> chunks[16][16];
  std::vector<unsigned> _chunk_instance_indices;
  std::vector<std::array<math::vector_4d, 9 * 9>> _chunk_data;
  std::vector<liquid_layer*> _chunk_layer_ptrs;
  std::vector<std::vector<int>> _chunk_layer_texture_samplers;
  std::size_t _n_chunk_instances = 256;

  opengl::scoped::deferred_upload_buffers<1> _buffers;

  std::vector<GLuint> _liquid_vertex_data_textures;
  std::vector<GLuint> _liquid_parameters_buffers;
  GLuint _liquid_chunk_index_texture = 0;

  unsigned _n_layer_chunks = 0;

  bool _need_buffer_update = false;
  bool _loaded = false;

  unsigned _update_flags;

  float xbase;
  float zbase;
};
