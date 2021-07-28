// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/DBCFile.h>
#include <noggit/MPQ.h>
#include <noggit/ModelInstance.h>
#include <noggit/ContextObject.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.fwd.hpp>

#include <memory>
#include <string>
#include <vector>


struct OutdoorLightStats
{
  float nightIntensity;
  math::vector_3d dayDir;

  void interpolate(OutdoorLightStats *a, OutdoorLightStats *b, float r);
};

class OutdoorLighting
{
private:
  std::vector<OutdoorLightStats> lightStats;

public:
  explicit OutdoorLighting(const std::string& fname);

  OutdoorLightStats getLightStats(int time);
};

struct SkyColor 
{
  math::vector_3d color;
  int time;

  SkyColor(int t, int col);
};

struct SkyFloatParam
{
  SkyFloatParam(int t, float val);

  float value;
  int time;
};

class Sky 
{
public:
  boost::optional<ModelInstance> skybox;

  math::vector_3d pos;
  float r1, r2;

  explicit Sky(DBCFile::Iterator data, noggit::NoggitRenderContext context);

  std::vector<SkyColor> colorRows[36];
  std::vector<SkyFloatParam> floatParams[6];
  int mmin[36];
  int mmin_float[6];

  char name[32];

  math::vector_3d colorFor(int r, int t) const;
  float floatParamFor(int r, int t) const;

  float weight;
  bool global;

  bool operator<(const Sky& s) const
  {
    if (global) return false;
    else if (s.global) return true;
    else return r2 < s.r2;
  }

  float river_shallow_alpha() const { return _river_shallow_alpha; }
  float river_deep_alpha() const { return _river_deep_alpha; }
  float ocean_shallow_alpha() const { return _ocean_shallow_alpha; }
  float ocean_deep_alpha() const { return _ocean_deep_alpha; }

private:
  float _river_shallow_alpha;
  float _river_deep_alpha;
  float _ocean_shallow_alpha;
  float _ocean_deep_alpha;

  noggit::NoggitRenderContext _context;
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
  UNK_1,
  UNK_2,
  NUM_SkyFloatParamsNames
};

class Skies 
{
private:
  int numSkies = 0;
  int cs = -1;
  ModelInstance stars;

  int _last_time = -1;
  math::vector_3d _last_pos;

  float _river_shallow_alpha;
  float _river_deep_alpha;
  float _ocean_shallow_alpha;
  float _ocean_deep_alpha;

  float _fog_distance;
  float _fog_multiplier;

public:
  std::vector<Sky> skies;
  std::vector<math::vector_3d> color_set = std::vector<math::vector_3d>(NUM_SkyColorNames);

  explicit Skies(unsigned int mapid, noggit::NoggitRenderContext context);

  Sky* findSkyWeights(math::vector_3d pos);
  void update_sky_colors(math::vector_3d pos, int time);

  bool draw ( math::matrix_4x4 const& model_view
            , math::matrix_4x4 const& projection
            , math::vector_3d const& camera_pos
            , opengl::scoped::use_program& m2_shader
            , math::frustum const& frustum
            , const float& cull_distance
            , int animtime
            , bool draw_particles
            , OutdoorLightStats const& light_stats
            );
  bool hasSkies() { return numSkies > 0; }

  float river_shallow_alpha() const { return _river_shallow_alpha; }
  float river_deep_alpha() const { return _river_deep_alpha; }
  float ocean_shallow_alpha() const { return _ocean_shallow_alpha; }
  float ocean_deep_alpha() const { return _ocean_deep_alpha; }

  float fog_distance_end() const { return _fog_distance / 36.f; };
  float fog_distance_start() const { return _fog_multiplier; };

  void unload();

private:
  bool _uploaded = false;
  bool _need_color_buffer_update = true;
  bool _need_vao_update = true;

  int _indices_count;

  void upload();
  void update_color_buffer();
  void update_vao(opengl::scoped::use_program& shader);

  opengl::scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
  opengl::scoped::deferred_upload_buffers<3> _buffers;
  GLuint const& _vertices_vbo = _buffers[0];
  GLuint const& _colors_vbo = _buffers[1];
  GLuint const& _indices_vbo = _buffers[2];

  std::unique_ptr<opengl::program> _program;

  noggit::NoggitRenderContext _context;
};
