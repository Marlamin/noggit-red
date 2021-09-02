// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <noggit/map_enums.hpp>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>
#include <noggit/Selection.h>
#include <noggit/TileWater.hpp>
#include <noggit/tile_index.hpp>
#include <noggit/tool_enums.hpp>
#include <opengl/shader.fwd.hpp>
#include <noggit/ContextObject.hpp>
#include <noggit/Misc.h>

#include <map>
#include <string>
#include <vector>
#include <array>

namespace math
{
  class frustum;
  struct vector_3d;
}

class World;

class MapTile : public AsyncObject
{
public:
	MapTile( int x0
         , int z0
         , std::string const& pFilename
         , bool pBigAlpha
         , bool pLoadModels
         , bool use_mclq_green_lava
         , bool reloading_tile
         , World*
         , noggit::NoggitRenderContext context
         , tile_mode mode = tile_mode::edit
         );
  ~MapTile();

  void finishLoading();

  //! \todo on destruction, unload ModelInstances and WMOInstances on this tile:
  // a) either keep up the information what tiles the instances are on at all times
  //    (even while moving), to then check if all tiles it was on were unloaded, or
  // b) do the reference count lazily by iterating over all instances and checking
  //    what MapTiles they span. if any of those tiles is still loaded, keep it,
  //    otherwise remove it.
  //
  // I think b) is easier. It only requires
  // `std::set<C2iVector> XInstance::spanning_tiles() const` followed by
  // `if_none (isTileLoaded (x, y)): unload instance`, which is way easier than
  // constantly updating the reference counters.
  // Note that both approaches do not cover the issue that the instance might not
  // be saved to any tile, thus the movement might have been lost.

	//! \brief Get the maximum height of terrain on this map tile.
	float getMaxHeight();
	float getMinHeight();

  void convert_alphamap(bool to_big_alpha);

  //! \brief Get chunk for sub offset x,z.
  MapChunk* getChunk(unsigned int x, unsigned int z);
  //! \todo map_index style iterators
  std::vector<MapChunk*> chunks_in_range (math::vector_3d const& pos, float radius) const;
  std::vector<MapChunk*> chunks_in_rect (math::vector_3d const& pos, float radius) const;

  const tile_index index;
  float xbase, zbase;

  std::atomic<bool> changed;

  void draw ( math::frustum const& frustum
            , opengl::scoped::use_program& mcnk_shader
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool need_visibility_update
            , bool show_unpaintable_chunks
            , bool draw_paintability_overlay
            , bool draw_chunk_flag_overlay
            , bool draw_areaid_overlay
            , std::map<int, misc::random_color>& area_id_colors
            , int animtime
            , display_mode display
            , bool is_selected
            );
  bool intersect (math::ray const&, selection_result*) const;
  void drawWater ( math::frustum const& frustum
                 , const float& cull_distance
                 , const math::vector_3d& camera
                 , bool camera_moved
                 , liquid_render& render
                 , opengl::scoped::use_program& water_shader
                 , int animtime
                 , int layer
                 , display_mode display
                 );

  void drawMFBO (opengl::scoped::use_program&);

  bool GetVertex(float x, float z, math::vector_3d *V);
  void getVertexInternal(float x, float z, math::vector_3d* v);

  void saveTile(World*);
	void CropWater();

  bool isTile(int pX, int pZ);

  virtual async_priority loading_priority() const
  {
    return async_priority::high;
  }

  bool has_model(uint32_t uid) const
  {
    return std::find(uids.begin(), uids.end(), uid) != uids.end();
  }

  void remove_model(uint32_t uid);
  void add_model(uint32_t uid);

  TileWater Water;

  bool tile_is_being_reloaded() const { return _tile_is_being_reloaded; }

  std::vector<uint32_t>* get_uids() { return &uids; }

  void initEmptyChunks();

  void setFilename(const std::string& new_filename) {filename = new_filename;};

  QImage getHeightmapImage(float min_height, float max_height);
  QImage getAlphamapImage(unsigned layer);
  QImage getAlphamapImage(std::string const& filename);
  QImage getVertexColorsImage();
  QImage getNormalmapImage();
  void setHeightmapImage(QImage const& image, float multiplier, int mode);
  void setAlphaImage(QImage const& image, unsigned layer);
  void setVertexColorImage(QImage const& image, int mode);
  void registerChunkUpdate(unsigned flags) { _chunk_update_flags |= flags; };
  void endChunkUpdates() { _chunk_update_flags = 0; };
  std::array<float, 145 * 256 * 4>& getChunkHeightmapBuffer() { return _chunk_heightmap_buffer; };
  unsigned getChunkUpdateFlags() { return _chunk_update_flags; }
  void recalcExtents(float min, float max);
  std::array<math::vector_3d, 2>& getExtents() { return _extents; };

  void unload();

  GLuint getAlphamapTextureHandle() { return _alphamap_tex; };

private:

  void uploadTextures();

  tile_mode _mode;
  bool _tile_is_being_reloaded;
  bool _uploaded = false;
  bool _selected = false;
  bool _split_drawcall = false;
  bool _requires_sampler_reset = true;

  std::array<math::vector_3d, 2> _extents;

  // MFBO:
  math::vector_3d mMinimumValues[3 * 3];
  math::vector_3d mMaximumValues[3 * 3];

  bool _mfbo_buffer_are_setup = false;
  opengl::scoped::deferred_upload_vertex_arrays<2> _mfbo_vaos;
  GLuint const& _mfbo_bottom_vao = _mfbo_vaos[0];
  GLuint const& _mfbo_top_vao = _mfbo_vaos[1];
  opengl::scoped::deferred_upload_buffers<3> _mfbo_vbos;
  GLuint const& _mfbo_bottom_vbo = _mfbo_vbos[0];
  GLuint const& _mfbo_top_vbo = _mfbo_vbos[1];
  GLuint const& _mfbo_indices = _mfbo_vbos[2];

  opengl::scoped::deferred_upload_textures<4> _chunk_texture_arrays;
  GLuint const& _height_tex = _chunk_texture_arrays[0];
  GLuint const& _mccv_tex = _chunk_texture_arrays[1];
  GLuint const& _shadowmap_tex = _chunk_texture_arrays[2];
  GLuint const& _alphamap_tex = _chunk_texture_arrays[3];

  opengl::scoped::deferred_upload_buffers<1> _buffers;

  GLuint const& _chunk_instance_data_ubo = _buffers[0];
  opengl::ChunkInstanceDataUniformBlock _chunk_instance_data[256];
  std::array<float, 145 * 256 * 4> _chunk_heightmap_buffer;

  unsigned _chunk_update_flags;

  std::vector<int> _samplers;

  // MHDR:
  int mFlags;
  bool mBigAlpha;

  // Data to be loaded and later unloaded.
  std::vector<std::string> mTextureFilenames;
  std::vector<std::string> mModelFilenames;
  std::vector<std::string> mWMOFilenames;
  
  std::vector<uint32_t> uids;

  std::unique_ptr<MapChunk> mChunks[16][16];
  std::vector<TileWater*> chunksLiquids; //map chunks liquids for old style water render!!! (Not MH2O)

  bool _load_models;
  World* _world;

  noggit::NoggitRenderContext _context;

  friend class MapChunk;
  friend class TextureSet;
};
