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

constexpr int DAY_DURATION = 2880; // Time Values from 0 to 2880 where each number represents a half minute from midnight to midnight

// 3.3.5 only
enum SkyColorNames
{
  LIGHT_GLOBAL_DIFFUSE,
  LIGHT_GLOBAL_AMBIENT,
  SKY_COLOR_TOP, // top
  SKY_COLOR_MIDDLE, // middle
  SKY_COLOR_BAND1, // middle to horizon
  SKY_COLOR_BAND2, // above horizon
  SKY_COLOR_SMOG, // horizon/smog
  SKY_FOG_COLOR, // fog and WDL mountains
  SHADOW_OPACITY, // Unknown / unused in 3.3.5 ? This value was moved to ShadowOpacity(17) in the new format
  SUN_COLOR, // sun, specular light, sunrays
  SUN_CLOUD_COLOR, // bigger sun halo
  CLOUD_EMISSIVE_COLOR, // cloud edge
  CLOUD_LAYER1_AMBIENT_COLOR, // cloud body
  CLOUD_LAYER2_AMBIENT_COLOR, // Unknown / unused in 3.3.5 ? This value was ported to Cloud Layer 2 Ambient Color in the new format
  OCEAN_COLOR_LIGHT, // shallow ocean
  OCEAN_COLOR_DARK, // deep ocean
  RIVER_COLOR_LIGHT, // shallow river
  RIVER_COLOR_DARK, // deep river
  NUM_SkyColorNames
};

enum SkyFloatParamsNames
{
  SKY_FOG_DISTANCE,
  SKY_FOG_MULTIPLIER,
  SKY_CELESTIAL_GLOW,
  SKY_CLOUD_DENSITY,
  SKY_UNK_FLOAT_PARAM_4,
  SKY_UNK_FLOAT_PARAM_5,
  NUM_SkyFloatParamsNames
};

enum SkyParamsNames
{
  SKY_PARAM_CLEAR,
  SKY_PARAM_CLEAR_UNDERWATER,
  SKY_PARAM_TORM,
  SKY_PARAM_STORM_UNDERWATER,
  SKY_PARAM_DEATH,
  SKY_PARAM_UNK_1,
  SKY_PARAM_UNK_2,
  SKY_PARAM_UNK_3,
  NUM_SkyParamsNames
};

enum SkyModelSkyBoxFlags
{
  LIGHT_SKYBOX_FULL_DAY = 0x1, // Full day Skybox: animation syncs with time of day (uses animation 0, time of day is just in percentage).
  LIGHT_SKYBOX_COMBINE = 0x2, // Combine Procedural And Skybox : render stars, sun and moons and clouds as well
  /*  
  0x04	Procedural Fog Color Blend
  0x08	Force Sun-shafts
  0x10	Disable use Sun Fog Color
  */
};

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

struct ZoneLight
{
  unsigned int id = 0;
  std::string name;
  // unsigned int mapId = 0; // map.dbc
  unsigned int lightId = 0; // light.dbc reference

  std::vector< glm::vec2> points;

  // added in 8.2
  // float zMin = -64000.0f;
  // float zMin = 64000.0f;

  std::array<glm::vec2, 2> _extents; // 2d square corners for fast bounds check before precise polygon intersection check
  // math::aabb_2d extents;

  // Sky* light = nullptr; 
  // std::vector<ZoneLightPoint> points;
};

struct ZoneLightPoint
{
    unsigned int id = 0;
    unsigned int zoneLightId = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    unsigned int pointOrder;
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

// TODO modern LightData.db
// unified timestamps for float and int data
// old data should fit in it as it does in classic
/*
struct LightData
{
  LightData(int paramId);
  
  unsigned int paramId;
  unsigned int time = 0;
  glm::vec3 colorRows[NUM_SkyColorNames] = {};
  float floatParams[NUM_SkyParamsNames] = {};
};*/

class SkyParam
{
public:
    std::optional<ModelInstance> skybox;
    int skyboxFlags = 0;

    int Id;

    SkyParam() = default;
    explicit SkyParam(int paramId, Noggit::NoggitRenderContext context);

    //array of 18 vectors(for each color), each vector item is a time/value. There can only be up to 16 vector items
    std::vector<SkyColor> colorRows[NUM_SkyColorNames];
    std::vector<SkyFloatParam> floatParams[NUM_SkyFloatParamsNames];

    // potential structure rework, more similar to retail/classic LightData.db
    // std::vector<LightData> lightData;

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

    // always save them for now
    // later we can have a system to only save modified dbcs
    bool _need_save = true;
    bool _colors_need_save = true;
    bool _floats_need_save = true;
    bool _is_new_param_record = false;

private: 
    // most common settings
    bool _highlight_sky = false;
    float _river_shallow_alpha = 0.5f;
    float _river_deep_alpha = 1.0f;
    float _ocean_shallow_alpha = 0.75f;
    float _ocean_deep_alpha = 1.0f;
    float _glow = 0.5f;
    // int _cloud_type = 0; // always 0 in 3.3.5

    Noggit::NoggitRenderContext _context;
};

class Sky 
{
private:
  mutable SkyParam* cachedCurrentParam = nullptr;

public:
  // std::optional<ModelInstance> skybox;
  int Id;
  glm::vec3 pos = glm::vec3(0, 0, 0);
  float r1 = 0.f, r2 = 0.f;
  std::string name;

  explicit Sky(DBCFile::Iterator data, Noggit::NoggitRenderContext context);

  int getId() const { return Id; };

  // std::unique_ptr<SkyParam> skyParams[NUM_SkyParamsNames];
  unsigned int skyParams[NUM_SkyParamsNames];
  int curr_sky_param = SKY_PARAM_CLEAR;


  std::optional<SkyParam*> getParam(int param_index) const;
  std::optional<SkyParam*> getCurrentParam() const { return getParam(curr_sky_param);};


  glm::vec3 colorFor(int r, int t) const;
  float floatParamFor(int r, int t) const;

  float weight = 0.0f;
  bool global = false;
  bool zone_light = false;

  bool is_new_record = false;

  bool operator<(const Sky& s) const
  {
    if (global) return false;
    else if (s.global) return true;
    else return r2 < s.r2;
  }

  bool selected() const { return _selected; }

  void save_to_dbc();

private:
  bool _selected;

  int mapId; // just for saving...

  Noggit::NoggitRenderContext _context;
};

class Skies 
{
private:
  void loadZoneLights(int map_id);

  int numSkies = 0;
  int cs = -1;
  ModelInstance stars;

  bool _force_update = true;
  int _last_time = -1;
  glm::vec3 _last_pos;

  // active render settings after blending between active lights
  // Look at Sky for individual light settings

  float _river_shallow_alpha = 0.5f;
  float _river_deep_alpha = 1.0f;
  float _ocean_shallow_alpha = 0.75f;
  float _ocean_deep_alpha = 1.0f;
  float _glow = 0.5f;
  // bool _highlight_sky = false; // since it's a bool and it can't be blended, just get from highest prio sky

  float _fog_rate = 1.5f;

  // float params
  float _fog_distance = 6500.0f;
  float _fog_multiplier = 0.1f;
  float _celestial_glow = 1.0f;
  float _cloud_density = 1.0f;
  float _unknown_float_param4 = 1.0f;
  float _unknown_float_param5 = 1.0f;

public:
  // Light Zones
  // hardcoded in the client in 3.3.5, they were moved to a DBC in 4.0
  std::vector<ZoneLight> zoneLightsWotlk;
  // std::unordered_map<int, std::vector<ZoneLightPoint>> zoneLightPoints; // grouped by zoneLightId. <zoneLightId, points>
  // std::vector<ZoneLightPoint> zoneLightPoints = {
  //     {300, 60, 4215.8745f, 3269.2654f, 1},
  // };
  bool using_fallback_global = false; // if map doesn't have a global

  SkyParamsNames active_param;

  std::vector<Sky> skies;
  std::array<glm::vec3, NUM_SkyColorNames> color_set = { glm::vec3(1.f, 1.f, 1.f) };
  Sky* findSkyById(int sky_id);

  explicit Skies(unsigned int mapid, Noggit::NoggitRenderContext context);

  Sky* createNewSky(Sky* old_sky, unsigned int new_id, glm::vec3& pos);

  Sky* findSkyWeights(glm::vec3 pos);

  Sky* findClosestSkyByWeight();
  Sky* findClosestSkyByDistance(glm::vec3 pos);

  void setCurrentParam(int param_id);
  void update_sky_colors(glm::vec3 pos, int time, bool global_only);

  bool draw ( glm::mat4x4 const& model_view
            , glm::mat4x4 const& projection
            , glm::vec3 const& camera_pos
            , OpenGL::Scoped::use_program& m2_shader
            , math::frustum const& frustum
            , const float& cull_distance
            , int animtime
            , int time
            /*, bool draw_particles*/
            , bool draw_skybox
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
  float fog_distance_start() const { return (_fog_distance / 36.f) * _fog_multiplier; };
  float fog_distance_multiplier() const { return _fog_multiplier; };

  float celestial_glow() const { return _celestial_glow; };
  float cloud_density() const { return _cloud_density; };
  float unknown_float_param4() const { return _unknown_float_param4; };
  float unknown_float_param5() const { return _unknown_float_param5; };

  float glow() const { return _glow; };

  float fogRate() const { return _fog_rate; }

  void unload();

  void force_update() { _force_update = true; }

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
