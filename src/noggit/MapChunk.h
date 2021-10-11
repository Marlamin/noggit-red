// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.hpp> // math::vector_4d
#include <noggit/map_enums.hpp>
#include <noggit/MapTile.h> // MapTile
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/TextureManager.h>
#include <noggit/WMOInstance.h>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ContextObject.hpp>
#include <opengl/scoped.hpp>
#include <opengl/texture.hpp>
#include <noggit/Misc.h>

#include <map>
#include <memory>
#include <array>
#include <QImage>

class MPQFile;
namespace math
{
  class frustum;
  struct vector_4d;
}
class Brush;
class ChunkWater;
class sExtendableArray;
class QPixmap;

using StripType = uint16_t;
static const int mapbufsize = 9 * 9 + 8 * 8; // chunk size

enum ChunkUpdateFlags
{
  VERTEX   = 0x1,
  SHADOW   = 0x2,
  MCCV     = 0x4,
  ALPHAMAP = 0x8,
  NORMALS  = 0x10,
  HOLES    = 0x20,
  AREA_ID  = 0x40,
  FLAGS    = 0x80
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
  MapChunk(MapTile* mt, MPQFile* f, bool bigAlpha, tile_mode mode, noggit::NoggitRenderContext context
           , bool init_empty = false, int chunk_idx = 0);

  auto getHoleMask(void) const -> unsigned { return static_cast<unsigned>(holes); }
  MapTile *mt;
  math::vector_3d vmin, vmax, vcenter;
  int px, py;

  MapChunkHeader header;

  float xbase, ybase, zbase;

  mcnk_flags header_flags;
  bool use_big_alphamap;

  std::unique_ptr<TextureSet> texture_set;

  int holes;
  bool currently_paintable = true;

  unsigned int areaID;

  math::vector_3d mVertices[mapbufsize];
  math::vector_3d mNormals[mapbufsize];
  math::vector_3d mccv[mapbufsize];

  uint8_t _shadow_map[64 * 64];

  void update_shadows();

  void unload();

  static int indexNoLoD(int x, int y);
  static int indexLoD(int x, int y);

private:

  unsigned _chunk_update_flags;

  noggit::NoggitRenderContext _context;

public:

  TextureSet* getTextureSet() { return texture_set.get(); };

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
            , std::array<int, 4>& textures_bound
            );
  //! \todo only this function should be public, all others should be called from it

  bool intersect (math::ray const&, selection_result*);
  bool ChangeMCCV(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode);
  bool stampMCCV(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode, QImage* img, bool paint, bool use_image_colors);
  math::vector_3d pickMCCV(math::vector_3d const& pos);

  ChunkWater* liquid_chunk() const;

  bool hasColors() const { return hasMCCV; };

  void updateVerticesData();
  void recalcNorms();
  void updateNormalsData();
  math::vector_3d getNeighborVertex(int i, unsigned dir);

  //! \todo implement Action stack for these
  bool changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius);
  bool flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode, const math::vector_3d& origin, math::degrees angle, math::degrees orientation);
  bool blurTerrain ( math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode
                   , std::function<boost::optional<float> (float, float)> height
                   );

  bool changeTerrainProcessVertex(math::vector_3d const& pos, math::vector_3d const& vertex, float& dt, float radiusOuter, float radiusInner, int brushType);
  auto stamp(math::vector_3d const& pos, float dt, QImage const* img, float radiusOuter
  , float radiusInner, int brushType, bool sculpt) -> void;
  void selectVertex(math::vector_3d const& pos, float radius, std::unordered_set<math::vector_3d*>& vertices);
  void fixVertices(std::unordered_set<math::vector_3d*>& selected);
  // for the vertex tool
  bool isBorderChunk(std::unordered_set<math::vector_3d*>& selected);

  //! \todo implement Action stack for these
  bool paintTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool stampTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture, QImage* img, bool paint);
  bool replaceTexture(math::vector_3d const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture);
  bool canPaintTexture(scoped_blp_texture_reference texture);
  int addTexture(scoped_blp_texture_reference texture);
  void switchTexture(scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture);
  void eraseTextures();
  void change_texture_flag(scoped_blp_texture_reference const& tex, std::size_t flag, bool add);

  void clear_shadows();

  bool isHole(int i, int j);
  void setHole(math::vector_3d const& pos, float radius, bool big, bool add);

  void setFlag(bool value, uint32_t);

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x, float z, math::vector_3d *V);
  void getVertexInternal(float x, float z, math::vector_3d * v);
  float getHeight(int x, int z);
  float getMinHeight();
  float getMaxHeight() { return vmin.y; };
  math::vector_3d getCenter() { return vcenter; };

  void clearHeight();

  //! \todo this is ugly create a build struct or sth
  void save(sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::vector<WMOInstance> &lObjectInstances, std::vector<ModelInstance>& lModelInstances);

  // fix the gaps with the chunk to the left
  bool fixGapLeft(const MapChunk* chunk);
  // fix the gaps with the chunk above
  bool fixGapAbove(const MapChunk* chunk);

  math::vector_3d* getHeightmap() { return &mVertices[0]; };
  math::vector_3d const* getNormals() const { return &mNormals[0]; }
  math::vector_3d* getVertexColors() { return &mccv[0]; };

  void update_vertex_colors();

  QImage getHeightmapImage(float min_height, float max_height);
  QImage getAlphamapImage(unsigned layer);
  QImage getVertexColorImage();
  void setHeightmapImage(QImage const& image, float multiplier, int mode);
  void setAlphamapImage(QImage const& image, unsigned layer);
  void setVertexColorImage(QImage const& image);

  void registerChunkUpdate(unsigned flags);
  void endChunkUpdates() { _chunk_update_flags = 0; }
  unsigned getUpdateFlags() { return _chunk_update_flags; }
};
