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

struct ZoneLight
{
  unsigned int id = 0;
  std::string name;
  unsigned int mapId = 0; // map.dbc
  unsigned int lightId = 0; // light.dbc reference
};

struct ZoneLightPoint
{
    unsigned int id = 0;
    unsigned int zoneLightId = 0;
    float pos1 = 0.0f;
    float pos2 = 0.0f;
    unsigned int pointOrder;
};

// hardcoded in the client in 3.3.5, they were moved to a DBC in 4.0
// TODO : move to a definition file
static const std::vector<ZoneLight> zoneLightsWotlk = {
    {60, "BoreanTundra", 571, 914},
    {61, "DragonBlight", 571, 825},
    {62, "GrizzlyHills", 571, 959},
    {63, "HowlingFjord", 571, 862},
    {64, "HowlingGrizzly", 571, 1847},
    {65, "Icecrown", 571, 1703},
    {66, "Sholazar", 571, 1796},
    {67, "StormPeaks", 571, 1777},
    {68, "Wintergrasp", 571, 1792},
    {69, "ZulDrak", 571, 1589},
    {70, "CrystalSong", 571, 1740},
    {187, "Shadow Moon Main", 530, 519}
};

// TODO also move to a csv file. added just borean tundra as example
/*
std::vector<ZoneLightPoint> zoneLightPoints = {
    {300, 60, 4215.8745f, 3269.2654f, 1},
    {301, 60, 4271.2856f, 4197.4040f, 2},
    {302, 60, 4363.6377f, 4262.0503f, 3},
    {303, 60, 4585.2827f, 4264.3594f, 4},
    {304, 60, 4562.1950f, 4617.6060f, 5},
    {305, 60, 4522.9450f, 4633.7670f, 6},
    {306, 60, 4520.6360f, 4675.3257f, 7},
    {307, 60, 4562.1950f, 4709.9580f, 8},
    {308, 60, 4566.8125f, 4936.2207f, 9},
    {309, 60, 4622.2236f, 4940.8380f, 10},
    {310, 60, 4631.4585f, 5499.5680f, 11},
    {311, 60, 4728.4280f, 5679.6550f, 12},
    {312, 60, 4728.4280f, 5956.7110f, 13},
    {313, 60, 4682.2524f, 6012.1226f, 14},
    {314, 60, 4673.0170f, 6487.7354f, 15},
    {315, 60, 6081.3860f, 8990.4780f, 16},
    {316, 60, 3043.0032f, 9590.7660f, 17},
    {317, 60, 826.5530f, 8247.0430f, 18},
    {318, 60, 133.91203f, 5448.7744f, 19},
    {319, 60, 957.1393f, 3272.8423f, 20},
    {320, 60, 2381.5320f, 3242.5560f, 21},
    {321, 60, 3117.2805f, 3195.2437f, 22},
    {322, 60, 3292.0650f, 3263.7822f, 23},
    {323, 60, 3610.3906f, 3122.0793f, 24},
    {324, 60, 3746.0330f, 3111.1125f, 25},
    {325, 60, 3812.3870f, 3198.4680f, 26},
    {326, 60, 4098.6855f, 3195.5237f, 27}
};
*/

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
