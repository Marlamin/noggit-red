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
#include <opengl/primitives.hpp>
#include <ClientFile.hpp>

#include <boost/optional.hpp>

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

struct WMORenderBatch
{
  std::uint32_t flags;
  std::uint32_t shader;
  std::uint32_t tex_array0;
  std::uint32_t tex_array1;
  std::uint32_t tex0;
  std::uint32_t tex1;
  std::uint32_t alpha_test_mode;
  std::uint32_t _pad1;
};

enum WMORenderBatchFlags
{
  eWMOBatch_ExteriorLit = 0x1,
  eWMOBatch_HasMOCV = 0x2,
  eWMOBatch_Unlit = 0x4,
  eWMOBatch_Unfogged = 0x8,
  eWMOBatch_Collision = 0x10
};

struct WMOCombinedDrawCall
{
  std::vector<int> samplers;
  std::uint32_t index_start = 0;
  std::uint32_t index_count = 0;
  std::uint32_t n_used_samplers = 0;
  bool backface_cull = false;
};

struct wmo_batch
{
  int8_t unused[12];

  uint32_t index_start;
  uint16_t index_count;
  uint16_t vertex_start;
  uint16_t vertex_end;

  uint8_t flags;
  uint8_t texture;
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
    uint32_t flag_0x100: 1;
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
public:
  WMOGroup(WMO *wmo, BlizzardArchive::ClientFile* f, int num, char const* names);
  WMOGroup(WMOGroup const&);

  void load();

  void draw( opengl::scoped::use_program& wmo_shader
           , math::frustum const& frustum
           , const float& cull_distance
           , const glm::vec3& camera
           , bool draw_fog
           , bool world_has_skies
           );

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
  bool is_visible( glm::mat4x4 const& transform_matrix
                 , math::frustum const& frustum
                 , float const& cull_distance
                 , glm::vec3 const& camera
                 , display_mode display
                 ) const;

  std::vector<uint16_t> doodad_ref() const { return _doodad_ref; }

  glm::vec3 BoundingBoxMin;
  glm::vec3 BoundingBoxMax;
  glm::vec3 VertexBoxMin;
  glm::vec3 VertexBoxMax;

  bool use_outdoor_lights;
  std::string name;

  bool has_skybox() const { return header.flags.skybox; }

  void unload();

private:
  void load_mocv(BlizzardArchive::ClientFile& f, uint32_t size);
  void fix_vertex_color_alpha();

  WMO *wmo;
  wmo_group_header header;
  ::glm::vec3 center;
  float rad;
  int32_t num;
  int32_t fog;
  std::vector<uint16_t> _doodad_ref;
  std::unique_ptr<wmo_liquid> lq;

  std::vector<wmo_batch> _batches;

  std::vector<::glm::vec3> _vertices;
  std::vector<::glm::vec3> _normals;
  std::vector<glm::vec2> _texcoords;
  std::vector<glm::vec2> _texcoords_2;
  std::vector<glm::vec4> _vertex_colors;
  std::vector<unsigned> _render_batch_mapping;
  std::vector<uint16_t> _indices;
  std::vector<WMORenderBatch> _render_batches;
  std::vector<WMOCombinedDrawCall> _draw_calls;

  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  opengl::scoped::deferred_upload_buffers<8> _buffers;
  GLuint const& _vertices_buffer = _buffers[0];
  GLuint const& _normals_buffer = _buffers[1];
  GLuint const& _texcoords_buffer = _buffers[2];
  GLuint const& _texcoords_buffer_2 = _buffers[3];
  GLuint const& _vertex_colors_buffer = _buffers[4];
  GLuint const& _indices_buffer = _buffers[5];
  GLuint const& _render_batch_mapping_buffer = _buffers[6];
  GLuint const& _render_batch_tex_buffer = _buffers[7];

  GLuint _render_batch_tex;

  bool _uploaded = false;
  bool _vao_is_setup = false;

  void upload();

  void setup_vao(opengl::scoped::use_program& wmo_shader);
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
public:
  explicit WMO(const std::string& name, noggit::NoggitRenderContext context );

  void draw ( opengl::scoped::use_program& wmo_shader
            , glm::mat4x4 const& model_view
            , glm::mat4x4 const& projection
            , glm::mat4x4 const& transform_matrix
            , bool boundingbox
            , math::frustum const& frustum
            , const float& cull_distance
            , const glm::vec3& camera
            , bool draw_doodads
            , bool draw_fog
            , int animtime
            , bool world_has_skies
            , display_mode display
            );

  bool draw_skybox(glm::mat4x4 const& model_view
                  , glm::vec3 const& camera_pos
                  , opengl::scoped::use_program& m2_shader
                  , math::frustum const& frustum
                  , const float& cull_distance
                  , int animtime
                  , bool draw_particles
                  , glm::vec3 aabb_min
                  , glm::vec3 aabb_max
                  , std::map<int, std::pair<glm::vec3, glm::vec3>> const& group_extents
                  ) const;

  std::vector<float> intersect (math::ray const&) const;

  void finishLoading();

  virtual void waitForChildrenLoaded() override;

  void unload();

  std::map<uint32_t, std::vector<wmo_doodad_instance>> doodads_per_group(uint16_t doodadset) const;

  bool draw_group_boundingboxes;

  bool _finished_upload;

  std::vector<WMOGroup> groups;
  std::vector<WMOMaterial> materials;
  glm::vec3 extents[2];
  std::vector<scoped_blp_texture_reference> textures;
  std::vector<std::string> models;
  std::vector<wmo_doodad_instance> modelis;
  std::vector<glm::vec3> model_nearest_light_vector;

  std::vector<WMOLight> lights;
  glm::vec4 ambient_light_color;

  mohd_flags flags;

  std::vector<WMOFog> fogs;

  std::vector<WMODoodadSet> doodadsets;

  boost::optional<scoped_model_reference> skybox;

  noggit::NoggitRenderContext _context;

  bool is_hidden() const { return _hidden; }
  void toggle_visibility() { _hidden = !_hidden; }
  void show() { _hidden = false ; }

  virtual bool is_required_when_saving() const
  {
    return true;
  }

private:
  bool _hidden = false;
};

class WMOManager
{
public:
  static void report();
  static void clear_hidden_wmos();
  static void unload_all(noggit::NoggitRenderContext context);
private:
  friend struct scoped_wmo_reference;
  static noggit::AsyncObjectMultimap<WMO> _;
};

struct scoped_wmo_reference
{
  scoped_wmo_reference (std::string const& filename, noggit::NoggitRenderContext context)
    : _valid(true)
    , _filename(filename)
    , _context(context)
    , _wmo (WMOManager::_.emplace(_filename, context))
  {}

  scoped_wmo_reference (scoped_wmo_reference const& other)
    : _valid(other._valid)
    , _filename(other._filename)
    , _wmo(WMOManager::_.emplace(_filename, other._context))
    , _context(other._context)
  {}
  scoped_wmo_reference& operator= (scoped_wmo_reference const& other)
  {
    _valid = other._valid;
    _filename = other._filename;
    _wmo = WMOManager::_.emplace(_filename, other._context);
    _context = other._context;
    return *this;
  }

  scoped_wmo_reference (scoped_wmo_reference&& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _wmo (other._wmo)
    , _context (other._context)
  {
    other._valid = false;
  }
  scoped_wmo_reference& operator= (scoped_wmo_reference&& other)
  {
    std::swap(_valid, other._valid);
    std::swap(_filename, other._filename);
    std::swap(_wmo, other._wmo);
    std::swap(_context, other._context);
    other._valid = false;
    return *this;
  }

  ~scoped_wmo_reference()
  {
    if (_valid)
    {
      WMOManager::_.erase(_filename, _context);
    }
  }

  WMO* operator->() const
  {
    return _wmo;
  }
  WMO* get() const
  {
    return _wmo;
  }

private:  
  bool _valid;

  std::string _filename;
  WMO* _wmo;
  noggit::NoggitRenderContext _context;
};
