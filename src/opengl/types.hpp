// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtGui/QOpenGLContext>
#include <glm/mat4x4.hpp>
#include <cstdint>
#include <array>

namespace OpenGL
{
  typedef GLuint light;

  enum ubo_targets
  {
    MVP,
    LIGHTING,
    TERRAIN_OVERLAYS,
    CHUNK_INSTANCE_DATA,
    CHUNK_LIQUID_INSTANCE_INDEX
  };

  struct MVPUniformBlock
  {
  	glm::mat4x4 model_view;
    glm::mat4x4 projection;
  };

  struct LightingUniformBlock
  {
    glm::vec4 DiffuseColor_FogStart;
    glm::vec4 AmbientColor_FogEnd;
    glm::vec4 FogColor_FogOn;
    glm::vec4 LightDir_FogRate;
    glm::vec4 OceanColorLight;
    glm::vec4 OceanColorDark;
    glm::vec4 RiverColorLight;
    glm::vec4 RiverColorDark;
  };

  struct TerrainParamsUniformBlock
  {
    int draw_shadows = true;
    int draw_lines = false;
    int draw_hole_lines = false;
    int draw_areaid_overlay = false;
    int draw_terrain_height_contour = false;
    int draw_wireframe = false;
    int wireframe_type;
    float wireframe_radius;
    float wireframe_width;
    int draw_impass_overlay = false;
    int draw_paintability_overlay = false;
    int draw_selection_overlay = false;
    glm::vec4 wireframe_color;
    int draw_impassible_climb = false;
    int climb_use_output_angle = false;
    int climb_use_smooth_interpolation = false;
    float climb_value;
    int draw_vertex_color = true;
    int padding[3];
  };

  struct ChunkInstanceDataUniformBlock
  {
    int ChunkTextureSamplers[4];
    int ChunkTextureArrayIDs[4];
    int ChunkHoles_DrawImpass_TexLayerCount_CantPaint[4];
    int ChunkTexDoAnim[4];
    int ChunkTexAnimSpeed[4];
    int AreaIDColor_Pad2_DrawSelection[4];
    int ChunkXZ_TileXZ[4];
    int ChunkTexAnimDir[4];
  };

  struct LiquidChunkInstanceDataUniformBlock
  {
    unsigned texture_array;
    unsigned type;
    float xbase;
    float zbase;
    float anim_u;
    float anim_v;
    unsigned subchunks_1;
    unsigned subchunks_2;
    unsigned n_texture_frames;
    unsigned _pad1;
    unsigned _pad2;
    unsigned _pad3;
    unsigned _pad4;
    unsigned _pad5;
    unsigned _pad6;
    unsigned _pad7;
  };

  struct M2RenderState
  {
    std::uint16_t blend = 0;
    bool backface_cull = true;
    bool z_buffered = false;
    bool unfogged = false;
    bool unlit = false;
    std::array<GLuint, 2> tex_arrays;
    std::array<GLuint, 2> tex_indices;
    std::array<GLint, 2> tex_unit_lookups;
    GLint pixel_shader = 0;

  };


}
