// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/frustum.hpp>
#include <math/trig.hpp>
#include <noggit/rendering/CursorRender.hpp>
#include <noggit/Misc.h>
#include <noggit/Model.h> // ModelManager
#include <noggit/Selection.h>
#include <noggit/Sky.h> // Skies, OutdoorLighting, OutdoorLightStats
#include <noggit/WMO.h> // WMOManager
#include <noggit/map_horizon.h>
#include <noggit/map_index.hpp>
#include <noggit/TileIndex.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/world_tile_update_queue.hpp>
#include <noggit/world_model_instances_storage.hpp>
#include <noggit/ui/MinimapCreator.hpp>
#include <noggit/ContextObject.hpp>
#include <noggit/rendering/Primitives.hpp>
#include <opengl/shader.fwd.hpp>
#include <opengl/types.hpp>
#include <noggit/rendering/LiquidTextureManager.hpp>
#include <optional>
#include <QtCore/QSettings>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include <array>
#include <noggit/project/ApplicationProject.h>
#include <noggit/rendering/WorldRender.hpp>

namespace Noggit
{
  struct object_paste_params;
  struct VertexSelectionCache;

  namespace Rendering
  {
    class WorldRender;
  }
}

class Brush;
class MapTile;
class QPixmap;

static const float detail_size = 8.0f;

using StripType = uint16_t;


class World
{
  friend class Noggit::Rendering::WorldRender;

protected:
  std::vector<selection_type> _current_selection;
  std::unordered_map<std::string, std::vector<ModelInstance*>> _models_by_filename;
  Noggit::world_model_instances_storage _model_instance_storage;
  Noggit::world_tile_update_queue _tile_update_queue;

public:
  MapIndex mapIndex;
  Noggit::map_horizon horizon;

  // Temporary variables for loading a WMO, if we have a global WMO.
  std::string mWmoFilename;
  ENTRY_MODF mWmoEntry;

  unsigned int getMapID();

  // Time of the day.
  float animtime;
  float time;

  //! \brief Name of this map.
  std::string basename;

  explicit World(const std::string& name, int map_id, Noggit::NoggitRenderContext context, bool create_empty = false);

  void setBasename(const std::string& name);

  SceneObject* getObjectInstance(std::uint32_t uid);

  void update_models_emitters(float dt);

  unsigned int getAreaID (glm::vec3 const&);
  void setAreaID(glm::vec3 const& pos, int id, bool adt,  float radius = -1.0f);

  Noggit::NoggitRenderContext getRenderContext() { return _context; };

  selection_result intersect (glm::mat4x4 const& model_view
                             , math::ray const&
                             , bool only_map
                             , bool do_objects
                             , bool draw_terrain
                             , bool draw_wmo
                             , bool draw_models
                             , bool draw_hidden_models
                             , bool draw_wmo_exterior
                             );

  MapChunk* getChunkAt(glm::vec3 const& pos);

  bool isInIndoorWmoGroup(std::array<glm::vec3, 2> obj_bounds, glm::mat4x4 obj_transform);

protected:
  // Information about the currently selected model / WMO / triangle.
  int _selected_model_count = 0;
  std::optional<glm::vec3> _multi_select_pivot;
public:

  Noggit::Rendering::WorldRender* renderer() { return &_renderer; }

  void update_selection_pivot();
  std::optional<glm::vec3> const& multi_select_pivot() const { return _multi_select_pivot; }

  // Selection related methods.
  bool is_selected(selection_type selection) const;
  bool is_selected(std::uint32_t uid) const;
  std::vector<selection_type> const& current_selection() const { return _current_selection; }
  std::optional<selection_type> get_last_selected_model() const;
  bool has_selection() const { return !_current_selection.empty(); }
  bool has_multiple_model_selected() const { return _selected_model_count > 1; }
  int get_selected_model_count() const { return _selected_model_count; }
  void set_current_selection(selection_type entry);
  void add_to_selection(selection_type entry);
  void remove_from_selection(selection_type entry);
  void remove_from_selection(std::uint32_t uid);
  void reset_selection();
  void delete_selected_models();
  glm::vec3 get_ground_height(glm::vec3 pos);
  void range_add_to_selection(glm::vec3 const& pos, float radius, bool remove);
  Noggit::world_model_instances_storage& getModelInstanceStorage() { return _model_instance_storage; };

  enum class m2_scaling_type
  {
    set,
    add,
    mult
  };

  void snap_selected_models_to_the_ground();
  void scale_selected_models(float v, m2_scaling_type type);
  void move_selected_models(float dx, float dy, float dz);
  void move_model(selection_type entry, float dx, float dy, float dz);
  void move_selected_models(glm::vec3 const& delta)
  {
    move_selected_models(delta.x, delta.y, delta.z);
  }
  void set_selected_models_pos(float x, float y, float z, bool change_height = true)
  {
    return set_selected_models_pos({x,y,z}, change_height);
  }
  void set_selected_models_pos(glm::vec3 const& pos, bool change_height = true);
  void set_model_pos(selection_type entry, glm::vec3 const& pos, bool change_height = true);
  void rotate_selected_models(math::degrees rx, math::degrees ry, math::degrees rz, bool use_pivot);
  void rotate_selected_models_randomly(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
  void set_selected_models_rotation(math::degrees rx, math::degrees ry, math::degrees rz);

  // Checks the normal of the terrain on model origin and rotates to that spot.
  void rotate_selected_models_to_ground_normal(bool smoothNormals);

  bool GetVertex(float x, float z, glm::vec3 *V) const;

  // check if the cursor is under map or in an unloaded tile
  bool isUnderMap(glm::vec3 const& pos);

  template<typename Fun>
  bool for_all_chunks_in_range ( glm::vec3 const& pos
                               , float radius
                               , Fun&& /* MapChunk* -> bool changed */
                               );
  template<typename Fun, typename Post>
  bool for_all_chunks_in_range ( glm::vec3 const& pos
                               , float radius
                               , Fun&& /* MapChunk* -> bool changed */
                               , Post&& /* MapChunk* -> void; called for all changed chunks */
                               );

  template<typename Fun>
  bool for_all_chunks_in_rect ( glm::vec3 const& pos
    , float radius
    , Fun&& /* MapChunk* -> bool changed */
  );

  template<typename Fun, typename Post>
  bool for_all_chunks_in_rect (glm::vec3 const& pos
    , float radius
    , Fun&& /* MapChunk* -> bool changed */
    , Post&& /* MapChunk* -> void; called for all changed chunks */
  );

  template<typename Fun>
    void for_all_chunks_on_tile (glm::vec3 const& pos, Fun&&);

    template<typename Fun>
    void for_all_chunks_on_tile(MapTile* tile, Fun&& fun);

  template<typename Fun>
    void for_chunk_at(glm::vec3 const& pos, Fun&& fun);
  template<typename Fun>
    auto for_maybe_chunk_at (glm::vec3 const& pos, Fun&& fun) -> std::optional<decltype (fun (nullptr))>;

  template<typename Fun>
    void for_tile_at(const TileIndex& pos, Fun&&);

  template<typename Fun>
    void for_tile_at_force(const TileIndex& pos, Fun&&);

  void changeObjectsWithTerrain(glm::vec3 const& pos, float change, float radius, int BrushType, float inner_radius, bool iter_wmos_ = true, bool iter_m2s = true);
  void changeTerrain(glm::vec3 const& pos, float change, float radius, int BrushType, float inner_radius);
  std::vector<selected_object_type> getObjectsInRange(glm::vec3 const& pos, float radius, bool ignore_height = true, bool iter_wmos_ = true, bool iter_m2s = true);
  void changeShader(glm::vec3 const& pos, glm::vec4 const& color, float change, float radius, bool editMode);
  void stampShader(glm::vec3 const& pos, glm::vec4 const& color, float change, float radius, bool editMode, QImage* img, bool paint, bool use_image_colors);
  glm::vec3 pickShaderColor(glm::vec3 const& pos);
  void flattenTerrain(glm::vec3 const& pos, float remain, float radius, int BrushType, flatten_mode const& mode, const glm::vec3& origin, math::degrees angle, math::degrees orientation);
  std::vector<std::pair<SceneObject*, float>> getObjectsGroundDistance(glm::vec3 const& pos, float radius, bool iter_wmos_, bool iter_m2s);
  void blurTerrain(glm::vec3 const& pos, float remain, float radius, int BrushType, flatten_mode const& mode);
  bool paintTexture(glm::vec3 const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool stampTexture(glm::vec3 const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture, QImage* img, bool paint);
  bool sprayTexture(glm::vec3 const& pos, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, scoped_blp_texture_reference texture);
  bool replaceTexture(glm::vec3 const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture, bool entire_chunk = false);

  void eraseTextures(glm::vec3 const& pos);
  void overwriteTextureAtCurrentChunk(glm::vec3 const& pos, scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture);
  void setBaseTexture(glm::vec3 const& pos);
  void clear_shadows(glm::vec3 const& pos);
  void clearTextures(glm::vec3 const& pos);
  void swapTexture(glm::vec3 const& pos, scoped_blp_texture_reference tex);
  void swapTextureGlobal(scoped_blp_texture_reference tex);
  void removeTexture(glm::vec3 const& pos, scoped_blp_texture_reference tex);
  void removeTexDuplicateOnADT(glm::vec3 const& pos);
  void change_texture_flag(glm::vec3 const& pos, scoped_blp_texture_reference const& tex, std::size_t flag, bool add);

  void setHole(glm::vec3 const& pos, float radius, bool big, bool hole);
  void setHoleADT(glm::vec3 const& pos, bool hole);

  void exportADTAlphamap(glm::vec3 const& pos);
  void exportADTNormalmap(glm::vec3 const& pos);
  void exportADTAlphamap(glm::vec3 const& pos, std::string const& filename);
  void exportADTHeightmap(glm::vec3 const& pos, float min_height, float max_height);
  void exportADTVertexColorMap(glm::vec3 const& pos);
  void exportAllADTsAlphamap();
  void exportAllADTsAlphamap(std::string const& filename);
  void exportAllADTsHeightmap();
  void exportAllADTsVertexColorMap();

  void importADTAlphamap(glm::vec3 const& pos, QImage const& image, unsigned layer);
  void importADTAlphamap(glm::vec3 const& pos);
  void importADTHeightmap(glm::vec3 const& pos, QImage const& image, float multiplier, unsigned mode, bool tiledEdges);
  void importADTHeightmap(glm::vec3 const& pos, float multiplier, unsigned mode, bool tiledEdges);
  void importADTVertexColorMap(glm::vec3 const& pos, int mode, bool tiledEdges);
  void importADTVertexColorMap(glm::vec3 const& pos, QImage const& image, int mode, bool tiledEdges);

  void importAllADTsAlphamaps();
  void importAllADTsHeightmaps(float multiplier, unsigned mode, bool tiledEdges);
  void importAllADTVertexColorMaps(unsigned mode, bool tiledEdges);

  void ensureAllTilesetsADT(glm::vec3 const& pos);
  void ensureAllTilesetsAllADTs();

  void notifyTileRendererOnSelectedTextureChange();

  void addM2 ( BlizzardArchive::Listfile::FileKey const& file_key
             , glm::vec3 newPos
             , float scale, math::degrees::vec3 rotation
             , Noggit::object_paste_params*
             );
  void addWMO ( BlizzardArchive::Listfile::FileKey const& file_key
              , glm::vec3 newPos
              , math::degrees::vec3 rotation
              );

  ModelInstance* addM2AndGetInstance ( BlizzardArchive::Listfile::FileKey const& file_key
      , glm::vec3 newPos
      , float scale, math::degrees::vec3 rotation
      , Noggit::object_paste_params*
  );

  WMOInstance* addWMOAndGetInstance ( BlizzardArchive::Listfile::FileKey const& file_key
      , glm::vec3 newPos
      , math::degrees::vec3 rotation
  );

  auto stamp(glm::vec3 const& pos, float dt, QImage const* img, float radiusOuter
  , float radiusInner, int BrushType, bool sculpt) -> void;

  // add a m2 instance to the world (needs to be positioned already), return the uid
  std::uint32_t add_model_instance(ModelInstance model_instance, bool from_reloading);
  // add a wmo instance to the world (needs to be positioned already), return the uid
  std::uint32_t add_wmo_instance(WMOInstance wmo_instance, bool from_reloading);

  std::optional<selection_type> get_model(std::uint32_t uid);
  void remove_models_if_needed(std::vector<uint32_t> const& uids);

  void reload_tile(TileIndex const& tile);

  void updateTilesEntry(selection_type const& entry, model_update type);
  void updateTilesEntry(SceneObject* entry, model_update type);
  void updateTilesWMO(WMOInstance* wmo, model_update type);
  void updateTilesModel(ModelInstance* m2, model_update type);
  void wait_for_all_tile_updates();

  void deleteModelInstance(int uid);
  void deleteWMOInstance(int uid);
  void deleteInstance(int uid);

  bool uid_duplicates_found() const;
  void delete_duplicate_model_and_wmo_instances();
  // used after the uid fix all
  void unload_every_model_and_wmo_instance();

	static bool IsEditableWorld(BlizzardDatabaseLib::Structures::BlizzardDatabaseRow& record);

    static bool IsWMOWorld(BlizzardDatabaseLib::Structures::BlizzardDatabaseRow& record);

  void clearHeight(glm::vec3 const& pos);
  void clearAllModelsOnADT(TileIndex const& tile);

  // liquids
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
                  , float opacity_factor
                  );
  void CropWaterADT(const TileIndex& pos);
  void setWaterType(const TileIndex& pos, int type, int layer);
  int getWaterType(const TileIndex& tile, int layer);
  void autoGenWaterTrans(const TileIndex&, float factor);


  void fixAllGaps();

  void convert_alphamap(bool to_big_alpha);

  bool deselectVertices(glm::vec3 const& pos, float radius);
  void selectVertices(glm::vec3 const& pos, float radius);
  void moveVertices(float h);
  void orientVertices ( glm::vec3 const& ref_pos
                      , math::degrees vertex_angle
                      , math::degrees vertex_orientation
                      );
  void flattenVertices (float height);

  void updateSelectedVertices();
  void updateVertexCenter();
  void clearVertexSelection();

  void deleteObjects(std::vector<selection_type> const& types);

  float getMaxTileHeight(const TileIndex& tile);

  glm::vec3 const& vertexCenter();

  void recalc_norms (MapChunk*) const;

  Noggit::VertexSelectionCache getVertexSelectionCache();
  void setVertexSelectionCache(Noggit::VertexSelectionCache& cache);

  bool need_model_updates = false;

  void loadAllTiles();
  unsigned getNumLoadedTiles() const { return _n_loaded_tiles; };
  unsigned getNumRenderedTiles() const { return _n_rendered_tiles; };

  void select_objects_in_area(
      const std::array<glm::vec2, 2> selection_box, 
      bool reset_selection,
      glm::mat4x4 view,
      glm::mat4x4 projection,
      int viewport_width,
      int viewport_height,
      float user_depth,
      glm::vec3 camera_position
  );

protected:
  void update_models_by_filename();

  std::unordered_set<MapChunk*>& vertexBorderChunks();

  std::unordered_set<MapTile*> _vertex_tiles;
  std::unordered_set<MapChunk*> _vertex_chunks;
  std::unordered_set<MapChunk*> _vertex_border_chunks;
  std::unordered_set<glm::vec3*> _vertices_selected;
  glm::vec3 _vertex_center;
  bool _vertex_center_updated = false;
  bool _vertex_border_updated = false;

  QSettings* _settings;

  Noggit::NoggitRenderContext _context;

  std::array<std::pair<std::pair<int, int>, MapTile*>, 64 * 64 > _loaded_tiles_buffer;

  Noggit::Rendering::WorldRender _renderer;

  // Debug metrics
  unsigned _n_loaded_tiles;
  unsigned _n_rendered_tiles;

};
