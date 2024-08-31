// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/map_enums.hpp>
#include <noggit/MapTile.h> // MapTile
#include <noggit/Misc.h>
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/TextureManager.h>
#include <noggit/WMOInstance.h>
#include <noggit/map_enums.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ContextObject.hpp>
#include <opengl/scoped.hpp>
#include <opengl/texture.hpp>
#include <util/sExtendableArray.hpp>

#include <optional>
#include <map>
#include <memory>
#include <array>
#include <QImage>

namespace BlizzardArchive
{
  class ClientFile;
}

namespace math
{
  class frustum;
  struct vector_4d;
}
class Brush;
class ChunkWater;
class QPixmap;

using StripType = uint16_t;
static const int mapbufsize = 9 * 9 + 8 * 8; // chunk size

enum ChunkUpdateFlags
{
  VERTEX        = 0x1,
  SHADOW        = 0x2,
  MCCV          = 0x4,
  ALPHAMAP      = 0x8,
  NORMALS       = 0x10,
  HOLES         = 0x20,
  AREA_ID       = 0x40,
  FLAGS         = 0x80, // both chunk and texture layers flags
  GROUND_EFFECT = 0x100,
  DETAILDOODADS_EXCLUSION = 0x200
};

class MapChunk
{
private:
  tile_mode _mode;

  bool hasMCCV;

  std::vector<uint8_t> compressed_shadow_map() const;
  bool has_shadows() const;

  void update_intersect_points();


public:
  MapChunk(MapTile* mt, BlizzardArchive::ClientFile* f, bool bigAlpha, tile_mode mode, Noggit::NoggitRenderContext context
           , bool init_empty = false, int chunk_idx = 0, bool load_textures = true);

  auto getHoleMask(void) const -> unsigned { return static_cast<unsigned>(holes); }
  MapTile *mt;
  glm::vec3 vmin, vmax, vcenter;
  int px, py;

  // MapChunkHeader header;
  // uint32_t nLayers = 0;

  float xbase, ybase, zbase; // global coords

  mcnk_flags header_flags;
  bool use_big_alphamap;

  std::unique_ptr<TextureSet> texture_set;

  int holes;
  bool currently_paintable = true;

  unsigned int areaID;

  std::vector<ENTRY_MCSE> sound_emitters;

  glm::vec3 mVertices[mapbufsize];
  glm::vec3 mNormals[mapbufsize];
  glm::vec3 mccv[mapbufsize]; // blizzard stores alpha, but deosn't seem to be used

  uint8_t _shadow_map[64 * 64];

  void update_shadows();

  void unload();

  static int indexNoLoD(int x, int y);
  static int indexLoD(int x, int y);

private:

  unsigned _chunk_update_flags;

  Noggit::NoggitRenderContext _context;

public:

    TextureSet* getTextureSet() const { return texture_set.get(); };

  void draw ( math::frustum const& frustum
            , OpenGL::Scoped::use_program& mcnk_shader
            , const float& cull_distance
            , const glm::vec3& camera
            , bool need_visibility_update
            , bool show_unpaintable_chunks
            , bool draw_paintability_overlay
            , bool draw_chunk_flag_overlay
            , bool draw_areaid_overlay
            , std::map<int, misc::random_color>& area_id_colors
            , int animtime
            , display_mode display
            , std::array<int, 4>& textures_bound
            );
  //! \todo only this function should be public, all others should be called from it

  bool intersect (math::ray const&, selection_result*);
  bool ChangeMCCV(glm::vec3 const& pos, glm::vec4 const& color, float change, float radius, bool editMode);
  bool stampMCCV(glm::vec3 const& pos, glm::vec4 const& color, float change, float radius, bool editMode, QImage* img, bool paint, bool use_image_colors);
  glm::vec3 pickMCCV(glm::vec3 const& pos);

  ChunkWater* liquid_chunk() const;

  bool hasColors() const { return hasMCCV; };

  void updateVerticesData();
  void recalcExtents();
  void recalcNorms();
  void updateNormalsData();
  glm::vec3 getNeighborVertex(int i, unsigned dir);

  glm::uvec2 getUnitIndextAt(glm::vec3 pos);

  //! \todo implement Action stack for these
  bool changeTerrain(glm::vec3 const& pos, float change, float radius, int BrushType, float inner_radius);
  bool flattenTerrain(glm::vec3 const& pos, float remain, float radius, int BrushType, flatten_mode const& mode, const glm::vec3& origin, math::degrees angle, math::degrees orientation);
  bool blurTerrain ( glm::vec3 const& pos, float remain, float radius, int BrushType, flatten_mode const& mode
                   /*, std::function<std::optional<float>(float, float)> height*/
                   );

  bool changeTerrainProcessVertex(glm::vec3 const& pos, glm::vec3 const& vertex, float& dt, float radiusOuter, float radiusInner, int brushType);
  auto stamp(glm::vec3 const& pos, float dt, QImage const* img, float radiusOuter
  , float radiusInner, int brushType, bool sculpt) -> void;
  void selectVertex(glm::vec3 const& pos, float radius, std::unordered_set<glm::vec3*>& vertices);
  void fixVertices(std::unordered_set<glm::vec3*>& selected);
  // for the vertex tool
  bool isBorderChunk(std::unordered_set<glm::vec3*>& selected);

  //! \todo implement Action stack for these
  bool paintTexture(glm::vec3 const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool stampTexture(glm::vec3 const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture, QImage* img, bool paint);
  bool replaceTexture(glm::vec3 const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture, bool entire_chunk = false);
  bool canPaintTexture(scoped_blp_texture_reference texture);
  int addTexture(scoped_blp_texture_reference texture);
  bool switchTexture(scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture);
  void eraseTextures();
  void eraseTexture(scoped_blp_texture_reference const& tex);
  void change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add);

  void clear_shadows();

  void paintDetailDoodadsExclusion(glm::vec3 const& pos, float radius, bool exclusion);

  bool isHole(int i, int j);
  void setHole(glm::vec3 const& pos, float radius, bool big, bool add);

  void setFlag(bool value, uint32_t);

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x, float z, glm::vec3 *V);
  void getVertexInternal(float x, float z, glm::vec3 * v);
  float getHeight(int x, int z);
  float getMinHeight() { return vmin.y; };
  float getMaxHeight() { return vmax.y; };
  glm::vec3 getCenter() { return vcenter; };

  void clearHeight();

  //! \todo this is ugly create a build struct or sth
  void save(util::sExtendableArray &lADTFile
            , int &lCurrentPosition
            , int &lMCIN_Position
            , std::map<std::string, int> &lTextures
            , std::vector<WMOInstance*> &lObjectInstances
            , std::vector<ModelInstance*>& lModelInstances);

  // fix the gaps with the chunk to the left
  bool fixGapLeft(const MapChunk* chunk);
  // fix the gaps with the chunk above
  bool fixGapAbove(const MapChunk* chunk);

  glm::vec3* getHeightmap() { return &mVertices[0]; };
  glm::vec3 const* getNormals() const { return &mNormals[0]; }
  glm::vec3* getVertexColors() { return &mccv[0]; };

  void update_vertex_colors();

  QImage getHeightmapImage(float min_height, float max_height);
  QImage getAlphamapImage(unsigned layer);
  QImage getVertexColorImage();
  void setHeightmapImage(QImage const& image, float multiplier, int mode);
  void setAlphamapImage(QImage const& image, unsigned layer);
  void setVertexColorImage(QImage const& image);
  void initMCCV();

  void registerChunkUpdate(unsigned flags);
  void endChunkUpdates() { _chunk_update_flags = 0; }
  unsigned getUpdateFlags() { return _chunk_update_flags; }
};
