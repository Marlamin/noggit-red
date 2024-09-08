// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <math/ray.hpp>

#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/ModelManager.h>
#include <noggit/AsyncObjectMultimap.hpp>
#include <noggit/TextureManager.h>
#include <noggit/tool_enums.hpp>
#include <noggit/wmo_liquid.hpp>
#include <noggit/ContextObject.hpp>
#include <noggit/rendering/WMOGroupRender.hpp>
#include <noggit/rendering/WMORender.hpp>
#include <noggit/rendering/Primitives.hpp>
#include <ClientFile.hpp>
#include <optional>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>

class WMO;
class WMOGroup;
class WMOInstance;
class WMOManager;
class wmo_liquid;
class Model;

namespace Noggit::Rendering
{
  class WMOGroupRender;
  class WMORender;
}


struct wmo_batch
{
  int16_t unused[6];

  uint32_t index_start;
  uint16_t index_count;
  uint16_t vertex_start;
  uint16_t vertex_end;

  uint8_t flags;
  uint8_t texture;
};

union wmo_mopy_flags
{
    int8_t value;
    struct
    {
        int8_t flag_0x01 : 1; // 0x1
        int8_t no_cam_collide : 1; // 0x2
        int8_t detail : 1; // 0x4
        int8_t collision : 1; // 0x8
        int8_t hint : 1;
        int8_t render : 1;
        int8_t flag_0x40 : 1; // 0x40
        int8_t collide_hit : 1; // 0x80

    };
};
static_assert (sizeof(wmo_mopy_flags) == sizeof(std::int8_t)
    , "bitfields shall be implemented packed"
    );

struct wmo_triangle_material_info
{
    wmo_mopy_flags flags;
    uint8_t texture;

    bool isTransFace() { return flags.flag_0x01 && (flags.detail || flags.render); }
    bool isColor() { return !flags.collision; }
    bool isRenderFace() { return flags.render && !flags.detail; }
    bool isCollidable() { return flags.collision || isRenderFace(); }

    bool isCollision() { return texture == 0xff; }
};

enum wmo_mobn_flags
{
    Flag_XAxis = 0x0,
    Flag_YAxis = 0x1,
    Flag_ZAxis = 0x2,
    Flag_AxisMask = 0x3,
    Flag_Leaf = 0x4,
    Flag_NoChild = 0xFFFF,
};

struct wmo_bsp_node
{
    uint16_t flags;
    int16_t negChild;      // index of bsp child node (right in this array)
    int16_t posChild;
    uint16_t nFaces;       // num of triangle faces in MOBR
    uint32_t faceStart;    // index of the first triangle index(in MOBR)
    float planeDist;
};

union wmo_group_flags
{
  uint32_t value;
  struct
  {
    uint32_t has_bsp_tree : 1; // 0x1
    uint32_t has_light_map : 1; // 0x2
    uint32_t has_vertex_color : 1; // 0x4
    uint32_t exterior : 1; // 0x8
    uint32_t flag_0x10 : 1;
    uint32_t flag_0x20 : 1;
    uint32_t exterior_lit : 1; // 0x40
    uint32_t unreacheable : 1; // 0x80
    uint32_t show_exterior_sky: 1;
    uint32_t has_light : 1; // 0x200
    uint32_t flag_0x400 : 1;
    uint32_t has_doodads : 1; // 0x800
    uint32_t has_water : 1; // 0x1000
    uint32_t indoor : 1; // 0x2000
    uint32_t flag_0x4000 : 1;
    uint32_t flag_0x8000 : 1;
    uint32_t always_draw : 1; // 0x10000
    uint32_t has_mori_morb : 1; // 0x20000, cata+ only (?)
    uint32_t skybox : 1; // 0x40000
    uint32_t ocean : 1; // 0x80000
    uint32_t flag_0x100000 : 1;
    uint32_t mount_allowed : 1; // 0x200000
    uint32_t flag_0x400000 : 1;
    uint32_t flag_0x800000 : 1;
    uint32_t use_mocv2_for_texture_blending : 1; // 0x1000000
    uint32_t has_two_motv : 1; // 0x2000000
    uint32_t antiportal : 1; // 0x4000000
    uint32_t unk : 1; // 0x8000000 requires intBatchCount == 0, extBatchCount == 0, UNREACHABLE. 
    uint32_t unused : 4;
  };
};
static_assert ( sizeof (wmo_group_flags) == sizeof (std::uint32_t)
              , "bitfields shall be implemented packed"
              );

struct wmo_group_header
{
  uint32_t group_name; // offset into MOGN
  uint32_t descriptive_group_name; // offset into MOGN
  wmo_group_flags flags;
  float box1[3];
  float box2[3];
  uint16_t portal_start;
  uint16_t portal_count;
  uint16_t transparency_batches_count;
  uint16_t interior_batch_count;
  uint16_t exterior_batch_count;
  uint16_t padding_or_batch_type_d; // probably padding, but might be data?
  uint8_t fogs[4];
  uint32_t group_liquid; // used for MLIQ
  uint32_t id;
  int32_t unk2, unk3;
};

class WMOGroup 
{
  friend class Noggit::Rendering::WMOGroupRender;

public:
  WMOGroup(WMO *wmo, BlizzardArchive::ClientFile* f, int num, char const* names);
  WMOGroup(WMOGroup const&);

  void load();

  /*
  void drawLiquid ( glm::mat4x4 const& transform
                  , liquid_render& render
                  , bool draw_fog
                  , int animtime
                  );

  */

  void setupFog (bool draw_fog, std::function<void (bool)> setup_fog);

  void intersect (math::ray const&, std::vector<float>* results) const;

  // todo: portal culling
  [[nodiscard]]
  bool is_visible( glm::mat4x4 const& transform_matrix
                 , math::frustum const& frustum
                 , float const& cull_distance
                 , glm::vec3 const& camera
                 , display_mode display
                 ) const;

  [[nodiscard]]
  std::vector<uint16_t> doodad_ref() const { return _doodad_ref; }

  glm::vec3 BoundingBoxMin;
  glm::vec3 BoundingBoxMax;
  glm::vec3 VertexBoxMin;
  glm::vec3 VertexBoxMax;

  bool use_outdoor_lights;
  std::string name;

  [[nodiscard]]
  bool has_skybox() const { return header.flags.skybox; }

  [[nodiscard]]
  bool is_indoor() const { return header.flags.indoor; }

  [[nodiscard]]
  Noggit::Rendering::WMOGroupRender* renderer() { return &_renderer; };
  ::glm::vec3 center;

private:
  void load_mocv(BlizzardArchive::ClientFile& f, uint32_t size);
  void fix_vertex_color_alpha();

  WMO *wmo;
  wmo_group_header header;
  float rad;
  int32_t num;
  int32_t fog;
  std::vector<uint16_t> _doodad_ref;
  std::unique_ptr<wmo_liquid> lq;

  std::vector <wmo_triangle_material_info> _material_infos;
  std::vector<wmo_batch> _batches;

  std::vector<::glm::vec3> _vertices;
  std::vector<::glm::vec3> _normals;
  std::vector<glm::vec2> _texcoords;
  std::vector<glm::vec2> _texcoords_2;
  std::vector<glm::vec4> _vertex_colors;
  std::vector<uint16_t> _indices;

  std::optional<std::vector<wmo_bsp_node>> _bsp_tree_nodes;
  std::optional<std::vector<uint16_t>> _bsp_indices;

  Noggit::Rendering::WMOGroupRender _renderer;
};

struct WMOLight {
  uint32_t flags, color;
  glm::vec3 pos;
  float intensity;
  float unk[5];
  float r;

  glm::vec4 fcolor;

  void init(BlizzardArchive::ClientFile* f);
  void setup(GLint light);

  static void setupOnce(GLint light, glm::vec3 dir, glm::vec3 light_color);
};

struct WMOPV {
  glm::vec3 a, b, c, d;
};

struct WMOPR {
  int16_t portal, group, dir, reserved;
};

struct WMODoodadSet {
  char name[0x14];
  int32_t start;
  int32_t size;
  int32_t unused;
};

struct WMOFog {
  unsigned int flags;
  glm::vec3 pos;
  float r1, r2, fogend, fogstart;
  unsigned int color1;
  float f2;
  float f3;
  unsigned int color2;
  // read to here (0x30 bytes)
  glm::vec4 color;
  void init(BlizzardArchive::ClientFile* f);
  void setup();
};

union mohd_flags
{
  std::uint16_t flags;
  struct
  {
    std::uint16_t do_not_attenuate_vertices_based_on_distance_to_portal : 1;
    std::uint16_t use_unified_render_path : 1;
    std::uint16_t use_liquid_type_dbc_id : 1;
    std::uint16_t do_not_fix_vertex_color_alpha : 1;
    std::uint16_t unused : 12;
  };
};
static_assert ( sizeof (mohd_flags) == sizeof (std::uint16_t)
              , "bitfields shall be implemented packed"
              );

class WMO : public AsyncObject
{
  friend class Noggit::Rendering::WMORender;

public:
  explicit WMO(BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context );

  [[nodiscard]]
  std::vector<float> intersect (math::ray const&, bool do_exterior = true) const;

  void finishLoading() override;

  void waitForChildrenLoaded() override;

  [[nodiscard]]
  std::map<uint32_t, std::vector<wmo_doodad_instance>> doodads_per_group(uint16_t doodadset) const;

  std::vector<WMOGroup> groups;
  std::vector<WMOMaterial> materials;
  glm::vec3 extents[2];
  std::vector<scoped_blp_texture_reference> textures;
  std::vector<std::string> models;
  std::vector<wmo_doodad_instance> modelis;
  std::vector<glm::vec3> model_nearest_light_vector;

  std::vector<WMOLight> lights;
  glm::vec4 ambient_light_color;

  uint32_t WmoId;

  mohd_flags flags;

  std::vector<WMOFog> fogs;

  std::vector<WMODoodadSet> doodadsets;

  std::optional<scoped_model_reference> skybox;

  Noggit::NoggitRenderContext _context;

  [[nodiscard]]
  bool is_hidden() const { return _hidden; }

  void toggle_visibility() { _hidden = !_hidden; }
  void show() { _hidden = false ; }
  void hide() { _hidden = true; }


  [[nodiscard]]
  bool is_required_when_saving()  const override
  {
    return true;
  }

  [[nodiscard]]
  Noggit::Rendering::WMORender* renderer() { return &_renderer; }

private:
  bool _hidden = false;

  Noggit::Rendering::WMORender _renderer;
};

class WMOManager
{
public:
  static void report();
  static void clear_hidden_wmos();
  static void unload_all(Noggit::NoggitRenderContext context);
private:
  friend struct scoped_wmo_reference;
  static Noggit::AsyncObjectMultimap<WMO> _;
};

struct scoped_wmo_reference
{
  scoped_wmo_reference (BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context)
    : _valid(true)
    , _file_key(file_key)
    , _context(context)
    , _wmo (WMOManager::_.emplace(file_key, context))
  {}

  scoped_wmo_reference (scoped_wmo_reference const& other)
    : _valid(other._valid)
    , _file_key(other._file_key)
    , _wmo(WMOManager::_.emplace(_file_key, other._context))
    , _context(other._context)
  {}
  scoped_wmo_reference& operator= (scoped_wmo_reference const& other)
  {
    _valid = other._valid;
    _file_key = other._file_key;
    _wmo = WMOManager::_.emplace(_file_key, other._context);
    _context = other._context;
    return *this;
  }

  scoped_wmo_reference (scoped_wmo_reference&& other)
    : _valid (other._valid)
    , _file_key (other._file_key)
    , _wmo (other._wmo)
    , _context (other._context)
  {
    other._valid = false;
  }
  scoped_wmo_reference& operator= (scoped_wmo_reference&& other)
  {
    std::swap(_valid, other._valid);
    std::swap(_file_key, other._file_key);
    std::swap(_wmo, other._wmo);
    std::swap(_context, other._context);
    other._valid = false;
    return *this;
  }

  ~scoped_wmo_reference()
  {
    if (_valid)
    {
      WMOManager::_.erase(_file_key, _context);
    }
  }

  WMO* operator->() const
  {
    return _wmo;
  }

  [[nodiscard]]
  WMO* get() const
  {
    return _wmo;
  }

private:  
  bool _valid;

  BlizzardArchive::Listfile::FileKey _file_key;
  WMO* _wmo;
  Noggit::NoggitRenderContext _context;
};
