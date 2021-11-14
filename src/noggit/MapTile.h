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
#include <external/tsl/robin_map.h>

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

struct MapTileDrawCall
{
  std::array<int, 11> samplers;
  unsigned start_chunk;
  unsigned n_chunks;
};

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
  std::vector<MapChunk*> chunks_in_range (glm::vec3 const& pos, float radius) const;
  std::vector<MapChunk*> chunks_in_rect (glm::vec3 const& pos, float radius) const;

  const tile_index index;
  float xbase, zbase;

  std::atomic<bool> changed;
  unsigned objects_frustum_cull_test = 0;
  bool tile_occluded = false;
  bool tile_frustum_culled = true;
  bool tile_occlusion_cull_override = true;

  void draw (opengl::scoped::use_program& mcnk_shader
            , const glm::vec3& camera
            , bool show_unpaintable_chunks
            , bool draw_paintability_overlay
            , bool is_selected
            );

  bool intersect (math::ray const&, selection_result*) const;
  void drawWater ( math::frustum const& frustum
                 , const float& cull_distance
                 , const glm::vec3& camera
                 , bool camera_moved
                 , opengl::scoped::use_program& water_shader
                 , int animtime
                 , int layer
                 , display_mode display
                 , LiquidTextureManager* tex_manager
                 );

  void drawMFBO (opengl::scoped::use_program&);

  bool GetVertex(float x, float z, glm::vec3 *V);
  void getVertexInternal(float x, float z, glm::vec3* v);

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
  void remove_model(SceneObject* instance);
  void add_model(uint32_t uid);
  void add_model(SceneObject* instance);

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
  void recalcExtents();
  void recalcObjectInstanceExtents();
  void recalcCombinedExtents();
  std::array<glm::vec3, 2>& getExtents() { return _extents; };
  std::array<glm::vec3, 2>& getCombinedExtents() { return _combined_extents; };

  void unload();

  GLuint getAlphamapTextureHandle() { return _alphamap_tex; };
  World* getWorld() { return _world; };

  void notifyTileRendererOnSelectedTextureChange() { _requires_paintability_recalc = true; };

  tsl::robin_map<AsyncObject*, std::vector<SceneObject*>> const& getObjectInstances() const { return object_instances; };

  void doTileOcclusionQuery(opengl::scoped::use_program& occlusion_shader);
  bool getTileOcclusionQueryResult(glm::vec3 const& camera);
  void discardTileOcclusionQuery() { _tile_occlusion_query_in_use = false; }

  float camDist() { return _cam_dist; }
  void calcCamDist(glm::vec3 const& camera);
  void markExtentsDirty() { _extents_dirty = true; }
  void tagCombinedExtents(bool state) { _combined_extents_dirty = state; };

private:

  void uploadTextures();
  bool fillSamplers(MapChunk* chunk, unsigned chunk_index, unsigned draw_call_index);

  tile_mode _mode;
  bool _tile_is_being_reloaded;
  bool _uploaded = false;
  bool _selected = false;
  bool _split_drawcall = false;
  bool _requires_sampler_reset = false;
  bool _requires_paintability_recalc = true;
  bool _requires_object_extents_recalc = true;
  bool _texture_not_loaded = false;
  bool _extents_dirty = true;
  bool _combined_extents_dirty = true;

  std::array<glm::vec3, 2> _extents;
  std::array<glm::vec3, 2> _object_instance_extents;
  std::array<glm::vec3, 2> _combined_extents;
  glm::vec3 _center;
  float _cam_dist;

  // MFBO:
  glm::vec3 mMinimumValues[3 * 3];
  glm::vec3 mMaximumValues[3 * 3];

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

  GLuint _tile_occlusion_query;
  bool _tile_occlusion_query_in_use = false;

  opengl::scoped::deferred_upload_buffers<1> _buffers;

  GLuint const& _chunk_instance_data_ubo = _buffers[0];
  opengl::ChunkInstanceDataUniformBlock _chunk_instance_data[256];
  std::array<float, 145 * 256 * 4> _chunk_heightmap_buffer;

  unsigned _chunk_update_flags;

  std::vector<MapTileDrawCall> _draw_calls;

  // MHDR:
  int mFlags;
  bool mBigAlpha;

  // Data to be loaded and later unloaded.
  std::vector<std::string> mTextureFilenames;
  std::vector<std::string> mModelFilenames;
  std::vector<std::string> mWMOFilenames;
  
  std::vector<uint32_t> uids;
  tsl::robin_map<AsyncObject*, std::vector<SceneObject*>> object_instances; // only includes M2 and WMO. perhaps a medium common ancestor then?

  std::unique_ptr<MapChunk> mChunks[16][16];

  bool _load_models;
  World* _world;

  noggit::NoggitRenderContext _context;

  friend class MapChunk;
  friend class TextureSet;
};
