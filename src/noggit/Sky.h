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

// 3.3.5 only
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

// modern LightData.db
// unified timestamps for float and int data
struct LightData
{
  LightData(int paramId);
  
  unsigned int paramId;
  unsigned int time = 0;
  glm::vec3 colorRows[NUM_SkyColorNames] = {};
  float floatParams[NUM_SkyParamsNames] = {};
};

class SkyParam
{
public:
    std::optional<ModelInstance> skybox;
    int skyboxFlags = 0;

    int Id;

    SkyParam() = default;
    explicit SkyParam(int paramId, Noggit::NoggitRenderContext context);

    //array of 18 vectors(for each color), each vector item is a time/value
    std::vector<SkyColor> colorRows[NUM_SkyColorNames];
    std::vector<SkyFloatParam> floatParams[NUM_SkyFloatParamsNames];

    // first/min time value for each entry
    // titi : deprecated those and replaced it by checking the time value of the first vector element
    // int mmin[NUM_SkyColorNames];
    // int mmin_float[NUM_SkyFloatParamsNames];

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

private: // most common settings
    bool _highlight_sky = false;
    float _river_shallow_alpha = 0.5f;
    float _river_deep_alpha = 1.0f;
    float _ocean_shallow_alpha = 0.75f;
    float _ocean_deep_alpha = 1.0f;
    float _glow = 0.5f;

    Noggit::NoggitRenderContext _context;
};

class Sky 
{
public:
  // std::optional<ModelInstance> skybox;

  int Id;
  int mapId; // just for saving...

  glm::vec3 pos = glm::vec3(0, 0, 0);
  float r1 = 0.f, r2 = 0.f;

  explicit Sky(DBCFile::Iterator data, Noggit::NoggitRenderContext context);

  SkyParam* skyParams[NUM_SkyParamsNames];
  int curr_sky_param = SKY_PARAM_CLEAR;

  std::string name;

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

  bool selected() const { return _selected; }

  void save_to_dbc();

private:
  bool _selected;

  Noggit::NoggitRenderContext _context;
};



class Skies 
{
private:
  int numSkies = 0;
  int cs = -1;
  ModelInstance stars;

  bool _force_update = true;
  int _last_time = -1;
  glm::vec3 _last_pos;

  float _river_shallow_alpha = 0.5f;
  float _river_deep_alpha = 1.0f;
  float _ocean_shallow_alpha = 0.75f;
  float _ocean_deep_alpha = 1.0f;
  float _glow = 0.5f;
  // bool _highlight_sky = false;

  float _fog_rate = 1.5f;

  float _fog_distance = 6500;
  float _fog_multiplier = 0.1;

public:
  std::vector<Sky> skies;
  std::vector<glm::vec3> color_set = std::vector<glm::vec3>(NUM_SkyColorNames, glm::vec3(1.f, 1.f, 1.f));

  explicit Skies(unsigned int mapid, Noggit::NoggitRenderContext context);

  Sky* createNewSky(Sky* old_sky, unsigned int new_id, glm::vec3& pos);

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
  float fog_distance_start() const { return _fog_multiplier; };

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
