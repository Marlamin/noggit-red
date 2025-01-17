// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncObject.h>
#include <noggit/ContextObject.hpp>
#include <noggit/map_enums.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/rendering/FlightBoundsRender.hpp>
#include <noggit/rendering/TileRender.hpp>
#include <noggit/Selection.h>
#include <noggit/TileIndex.hpp>
#include <noggit/TileWater.hpp>

#include <external/tsl/robin_map.h>

#include <array>
#include <map>
#include <string>
#include <vector>

namespace math
{
  class frustum;
  struct vector_3d;
  struct ray;
}

class MapChunk;
struct texture_heightmapping_data;
class World;


class MapTile : public AsyncObject
{
  friend class Noggit::Rendering::TileRender;
  friend class Noggit::Rendering::FlightBoundsRender;
  friend class MapChunk;
  friend class TextureSet;

public:
	MapTile( int x0
         , int z0
         , std::string const& pFilename
         , bool pBigAlpha
         , bool pLoadModels
         , bool use_mclq_green_lava
         , bool reloading_tile
         , World*
         , Noggit::NoggitRenderContext context
         , tile_mode mode = tile_mode::edit
         , bool pLoadTextures = true
         );
  ~MapTile();

  void finishLoading() override;
  void waitForChildrenLoaded() override;

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
  void forceRecalcExtents();

  void convert_alphamap(bool to_big_alpha);

  //! \brief Get chunk for sub offset x,z.

  [[nodiscard]]
  MapChunk* getChunk(unsigned int x, unsigned int z);
  //! \todo map_index style iterators

  [[nodiscard]]
  std::vector<MapChunk*> chunks_in_range (glm::vec3 const& pos, float radius) const;

  [[nodiscard]]
  std::vector<MapChunk*> chunks_in_rect (glm::vec3 const& pos, float radius) const;

  const TileIndex index;
  float xbase, zbase;

  std::atomic<bool> changed;

  bool _was_rendered_last_frame = false;

  bool intersect (math::ray const&, selection_result*);


  bool GetVertex(float x, float z, glm::vec3 *V);
  void getVertexInternal(float x, float z, glm::vec3* v);

	void CropWater();
  void saveTile(World* world);

private:
  void save(World* world, bool save_using_mclq_liquids);

public:

  bool isTile(int pX, int pZ);

  bool hasFlightBounds() const;;

  async_priority loading_priority() const override;

  bool has_model(uint32_t uid) const;

  void remove_model(uint32_t uid);
  void remove_model(SceneObject* instance);
  void add_model(uint32_t uid);
  void add_model(SceneObject* instance);

  TileWater Water;

  bool tile_is_being_reloaded() const;

  std::vector<uint32_t>* get_uids();

  void initEmptyChunks();

  void setFilename(const std::string& new_filename) {_file_key.setFilepath(new_filename);};

  QImage getHeightmapImage(float min_height, float max_height);
  QImage getAlphamapImage(unsigned layer);
  QImage getAlphamapImage(std::string const& filename);
  QImage getVertexColorsImage();
  QImage getNormalmapImage();
  void setHeightmapImage(QImage const& baseimage, float min_height, float max_height, int mode, bool tiledEdges);
  // void setWatermapImage(QImage const& baseimage, float multiplier, int mode, bool tiledEdges);
  void setAlphaImage(QImage const& image, unsigned layer, bool cleanup);
  void setVertexColorImage(QImage const& image, int mode, bool tiledEdges);
  void registerChunkUpdate(unsigned flags);;
  void endChunkUpdates();;
  std::array<float, 145 * 256 * 4>& getChunkHeightmapBuffer();;
  unsigned getChunkUpdateFlags() const;
  void recalcExtents();
  void recalcObjectInstanceExtents();
  void recalcCombinedExtents();
  std::array<glm::vec3, 2>& getExtents();;
  std::array<glm::vec3, 2>& getCombinedExtents();;

  World* getWorld();;

  [[nodiscard]]
  tsl::robin_map<AsyncObject*, std::vector<SceneObject*>> const& getObjectInstances() const;;

  float camDist() const;
  void calcCamDist(glm::vec3 const& camera);
  void markExtentsDirty();
  void tagCombinedExtents(bool state);;

  Noggit::Rendering::TileRender* renderer();;
  Noggit::Rendering::FlightBoundsRender* flightBoundsRenderer();;

  const texture_heightmapping_data& GetTextureHeightMappingData(const std::string& name) const;

  void forceAlphaUpdate();
  bool childrenFinishedLoading();
  bool texturesFinishedLoading();
  bool objectsFinishedLoading();

private:

  tile_mode _mode;
  bool _tile_is_being_reloaded;

  bool _extents_dirty = true;
  bool _combined_extents_dirty = true;
  bool _requires_object_extents_recalc = true;



  std::array<glm::vec3, 2> _extents;
  std::array<glm::vec3, 2> _object_instance_extents;
  std::array<glm::vec3, 2> _combined_extents;
  glm::vec3 _center;
  float _cam_dist;

  // MFBO: requires mFlags & 1
  glm::vec3 mMinimumValues[3 * 3] = {};
  glm::vec3 mMaximumValues[3 * 3] = {};

  unsigned _chunk_update_flags;

  bool _textures_finished_loading = false;
  bool _objects_finished_loading = false;

  // MHDR:
  int mFlags = 0;
  bool mBigAlpha;

  // Data to be loaded and later unloaded.
  std::vector<std::string> mTextureFilenames;
  // std::vector<std::string> mModelFilenames;
  // std::vector<std::string> mWMOFilenames;
  std::map<std::string, mtxf_entry> _mtxf_entries;
  
  std::vector<uint32_t> uids;
  tsl::robin_map<AsyncObject*, std::vector<SceneObject*>> object_instances; // only includes M2 and WMO. perhaps a medium common ancestor then?

  std::unique_ptr<MapChunk> mChunks[16][16];
  std::array<float, 145 * 256 * 4> _chunk_heightmap_buffer;

  bool _load_models;
  bool _load_textures;
  World* _world;

  Noggit::Rendering::TileRender _renderer;
  Noggit::Rendering::FlightBoundsRender _fl_bounds_render;

  Noggit::NoggitRenderContext _context;

};
