// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/TextureManager.h>
#include <opengl/scoped.hpp>

#include <memory>

namespace BlizzardArchive
{
  class ClientFile;
}

struct CImVector
{
  std::uint8_t b;
  std::uint8_t g;
  std::uint8_t r;
  std::uint8_t a;
};

struct CArgb
{
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
  std::uint8_t a;
};

struct SMOLTile
{
  uint8_t liquid : 6;
  uint8_t fishable : 1;
  uint8_t shared : 1;
};

struct WMOMaterial 
{
  union
  {
    uint32_t value;
    struct
    {
      uint32_t unlit :  1;
      uint32_t unfogged : 1;
      uint32_t unculled : 1;
      uint32_t ext_light: 1; // darkened used for the intern face of windows
      uint32_t sidn :  1; // night glow
      uint32_t window :  1; // lighting related(flag checked in CMapObj::UpdateSceneMaterials)
      uint32_t clamp_s :  1;
      uint32_t clamp_t : 1;
      uint32_t unused : 24;
    };
  } flags;
  uint32_t shader;
  uint32_t blend_mode; // Blending: 0 for opaque, 1 for transparent
  uint32_t texture_offset_1; // Start position for the first texture filename in the MOTX data block
  CImVector sidn_color; // emissive color
  CImVector frame_sidn_color; // runtime value
  uint32_t texture_offset_2; // Start position for the second texture filename in the MOTX data block
  CArgb diffuse_color;
  uint32_t ground_type;
  uint32_t texture_offset_3; 
  uint32_t color_2;
  uint32_t flag_2;
  uint32_t runtime_data[2];
  // also runtime data
  uint32_t texture1; // this is the first texture object.
  uint32_t texture2; // this is the second texture object.
};

struct WMOLiquidHeader {
  int32_t X, Y, A, B;
  glm::vec3 pos;
  int16_t material_id;
};

struct SMOWVert
{
  std::uint8_t flow1;
  std::uint8_t flow2;
  std::uint8_t flow1Pct;
  std::uint8_t filler;
};
struct SMOMVert
{
  std::int16_t s;
  std::int16_t t;
};

struct WmoLiquidVertex {
  union
  {
    SMOWVert water_vertex;
    SMOMVert magma_vertex;
  };
  float height;
};

class wmo_liquid
{
public:
  wmo_liquid(BlizzardArchive::ClientFile* f, WMOLiquidHeader const& header, int group_liquid, bool use_dbc_type, bool is_ocean);
  wmo_liquid(wmo_liquid const& other);

  void upload(OpenGL::Scoped::use_program& water_shader);

private:
  int initGeometry(BlizzardArchive::ClientFile* f);

  glm::vec3 pos;
  bool mTransparency;
  int xtiles, ytiles;
  int _liquid_id;

  std::vector<float> depths;
  std::vector<glm::vec2> tex_coords;
  std::vector<glm::vec3> vertices;
  std::vector<std::uint16_t> indices;

  int _indices_count;

  bool _uploaded = false;

  OpenGL::Scoped::deferred_upload_buffers<4> _buffer;
  GLuint const& _indices_buffer = _buffer[0];
  GLuint const& _vertices_buffer = _buffer[1];
  GLuint const& _depth_buffer = _buffer[2];
  GLuint const& _tex_coord_buffer = _buffer[3];
  OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vertex_array;
  GLuint const& _vao = _vertex_array[0];
};
