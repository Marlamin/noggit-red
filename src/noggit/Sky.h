// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/DBCFile.h>
#include <noggit/ModelInstance.h>
#include <noggit/ContextObject.hpp>
#include <noggit/rendering/Primitives.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.fwd.hpp>

#include <memory>
#include <string>
#include <vector>


struct OutdoorLightStats
{
  float nightIntensity;
  glm::vec3 dayDir;

  void interpolate(OutdoorLightStats *a, OutdoorLightStats *b, float r);
};

class OutdoorLighting
{
private:
  std::vector<OutdoorLightStats> lightStats;

public:
  OutdoorLighting();

  OutdoorLightStats getLightStats(int time);
};

struct SkyColor 
{
  glm::vec3 color;
  int time;

  SkyColor(int t, int col);
};

struct SkyFloatParam
{
  SkyFloatParam(int t, float val);

  float value;
  int time;
};

class SkyParam
{
public:
    std::optional<ModelInstance> skybox;
    int Id;

    SkyParam() = default;
    explicit SkyParam(int paramId, Noggit::NoggitRenderContext context);

    std::vector<SkyColor> colorRows[36];
    std::vector<SkyFloatParam> floatParams[6];
    int mmin[36];
    int mmin_float[6];

    bool highlight_sky() const { return _highlight_sky; }
    float river_shallow_alpha() const { return _river_shallow_alpha; }
    float river_deep_alpha() const { return _river_deep_alpha; }
    float ocean_shallow_alpha() const { return _ocean_shallow_alpha; }
    float ocean_deep_alpha() const { return _ocean_deep_alpha; }
    float glow() const { return _glow; }

    void set_glow(float glow) { _glow = glow; }
    void set_highlight_sky(bool state) { _highlight_sky = state; }
    void set_river_shallow_alpha(float alpha) { _river_shallow_alpha = alpha; }
    void set_river_deep_alpha(float alpha) { _river_deep_alpha = alpha; }
    void set_ocean_shallow_alpha(float alpha) { _ocean_shallow_alpha = alpha; }
    void set_ocean_deep_alpha(float alpha) { _ocean_deep_alpha = alpha; }

private:
    bool _highlight_sky;
    float _river_shallow_alpha;
    float _river_deep_alpha;
    float _ocean_shallow_alpha;
    float _ocean_deep_alpha;

    float _glow;

    Noggit::NoggitRenderContext _context;
};

class Sky 
{
public:
  std::optional<ModelInstance> skybox;

  int Id;
  glm::vec3 pos;
  float r1, r2;

  explicit Sky(DBCFile::Iterator data, Noggit::NoggitRenderContext context);

  SkyParam* skyParams[8];
  int curr_sky_param = 0;

  // std::vector<SkyColor> colorRows[36];
  // std::vector<SkyFloatParam> floatParams[6];
  // int mmin[36];
  // int mmin_float[6];

  char name[32];

  glm::vec3 colorFor(int r, int t) const;
  float floatParamFor(int r, int t) const;

  float weight;
  bool global;

  bool is_new_record = false;

  bool operator<(const Sky& s) const
  {
    if (global) return false;
    else if (s.global) return true;
    else return r2 < s.r2;
  }

  // bool highlight_sky() const { return _highlight_sky; }
  // float river_shallow_alpha() const { return _river_shallow_alpha; }
  // float river_deep_alpha() const { return _river_deep_alpha; }
  // float ocean_shallow_alpha() const { return _ocean_shallow_alpha; }
  // float ocean_deep_alpha() const { return _ocean_deep_alpha; }
  // float glow() const { return _glow; }
  bool selected() const { return _selected; }
  // 
  // void set_glow(float glow) { _glow = glow; }
  // void set_highlight_sky(bool state) { _highlight_sky = state; }
  // void set_river_shallow_alpha(float alpha) { _river_shallow_alpha = alpha; }
  // void set_river_deep_alpha(float alpha) { _river_deep_alpha = alpha; }
  // void set_ocean_shallow_alpha(float alpha) { _ocean_shallow_alpha = alpha; }
  // void set_ocean_deep_alpha(float alpha) { _ocean_deep_alpha = alpha; }

  void save_to_dbc();

private:
  // bool _highlight_sky;
  // float _river_shallow_alpha;
  // float _river_deep_alpha;
  // float _ocean_shallow_alpha;
  // float _ocean_deep_alpha;

  // float _glow;
  bool _selected;

  Noggit::NoggitRenderContext _context;
};

enum SkyColorNames 
{
  LIGHT_GLOBAL_DIFFUSE,
  LIGHT_GLOBAL_AMBIENT,
  SKY_COLOR_0, // top
  SKY_COLOR_1, // middle
  SKY_COLOR_2, // middle to horizon
  SKY_COLOR_3, // above horizon
  SKY_COLOR_4, // horizon
  FOG_COLOR, // fog and WDL mountains
  SHADOW_OPACITY,
  SUN_COLOR, // sun, specular light, sunrays
  SUN_HALO_COLOR, // bigger sun halo
  CLOUD_EDGE_COLOR, // cloud edge
  CLOUD_COLOR, // cloud body
  SKY_UNKNOWN_3,
  OCEAN_COLOR_LIGHT, // shallow ocean
  OCEAN_COLOR_DARK, // deep ocean
  RIVER_COLOR_LIGHT, // shallow river
  RIVER_COLOR_DARK, // deep river
  NUM_SkyColorNames
};

enum SkyFloatParamsNames
{
  FOG_DISTANCE,
  FOG_MULTIPLIER,
  CELESTIAL_FLOW,
  CLOUD_DENSITY,
  UNK_FLOAT_PARAM_1,
  UNK_FLOAT_PARAM_2,
  NUM_SkyFloatParamsNames
};

enum SkyParamsNames
{
    CLEAR,
    CLEAR_WATER,
    STORM,
    STORM_WATER,
    DEATH,
    UNK_PARAM_1,
    UNK_PARAM_2,
    UNK_PARAM_3,
    NUM_SkyParamsNames
};

class Skies 
{
private:
  int numSkies = 0;
  int cs = -1;
  ModelInstance stars;

  int _last_time = -1;
  glm::vec3 _last_pos;

  float _river_shallow_alpha;
  float _river_deep_alpha;
  float _ocean_shallow_alpha;
  float _ocean_deep_alpha;
  float _glow;
  float _fog_rate;

  float _fog_distance;
  float _fog_multiplier;

public:
  std::vector<Sky> skies;
  std::vector<glm::vec3> color_set = std::vector<glm::vec3>(NUM_SkyColorNames);

  explicit Skies(unsigned int mapid, Noggit::NoggitRenderContext context);

  Sky* findSkyWeights(glm::vec3 pos);

  Sky* findClosestSkyByWeight();
  Sky* findClosestSkyByDistance(glm::vec3 pos);

  void setCurrentParam(int param_id);
  void update_sky_colors(glm::vec3 pos, int time);

  bool draw ( glm::mat4x4 const& model_view
            , glm::mat4x4 const& projection
            , glm::vec3 const& camera_pos
            , OpenGL::Scoped::use_program& m2_shader
            , math::frustum const& frustum
            , const float& cull_distance
            , int animtime
            , OutdoorLightStats const& light_stats
            );

  void drawLightingSpheres (glm::mat4x4 const& model_view
                          , glm::mat4x4 const& projection
                          , glm::vec3 const& camera_pos
                          , math::frustum const& frustum
                          , const float& cull_distance
                          );

  void drawLightingSphereHandles (glm::mat4x4 const& model_view
                                , glm::mat4x4 const& projection
                                , glm::vec3 const& camera_pos
                                , math::frustum const& frustum
                                , const float& cull_distance
                                , bool draw_spheres
  );


  bool hasSkies() { return numSkies > 0; }

  float river_shallow_alpha() const { return _river_shallow_alpha; }
  float river_deep_alpha() const { return _river_deep_alpha; }
  float ocean_shallow_alpha() const { return _ocean_shallow_alpha; }
  float ocean_deep_alpha() const { return _ocean_deep_alpha; }

  float fog_distance_end() const { return _fog_distance / 36.f; };
  float fog_distance_start() const { return _fog_multiplier; };

  float glow() const { return _glow; };

  float fogRate() const { return _fog_rate; }

  void unload();

private:
  bool _uploaded = false;
  bool _need_color_buffer_update = true;
  bool _need_vao_update = true;

  int _indices_count;

  void upload();
  void update_color_buffer();
  void update_vao(OpenGL::Scoped::use_program& shader);

  OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  OpenGL::Scoped::deferred_upload_buffers<3> _buffers;
  GLuint const& _vertices_vbo = _buffers[0];
  GLuint const& _colors_vbo = _buffers[1];
  GLuint const& _indices_vbo = _buffers[2];

  std::unique_ptr<OpenGL::program> _program;

  Noggit::NoggitRenderContext _context;

  Noggit::Rendering::Primitives::Sphere _sphere_render;
};
