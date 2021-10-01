// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtGui/QOpenGLContext>
#include <math/matrix_4x4.hpp>
#include <math/vector_4d.hpp>

namespace opengl
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
    math::matrix_4x4 model_view;
    math::matrix_4x4 projection;
  };

  struct LightingUniformBlock
  {
    math::vector_4d DiffuseColor_FogStart;
    math::vector_4d AmbientColor_FogEnd;
    math::vector_4d FogColor_FogOn;
    math::vector_4d LightDir_FogRate;
    math::vector_4d OceanColorLight;
    math::vector_4d OceanColorDark;
    math::vector_4d RiverColorLight;
    math::vector_4d RiverColorDark;
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
    math::vector_4d wireframe_color;

  };

  struct ChunkInstanceDataUniformBlock
  {
    int ChunkTextureSamplers[4];
    int ChunkTextureArrayIDs[4];
    int ChunkHoles_DrawImpass_TexLayerCount_CantPaint[4];
    int ChunkTexDoAnim[4];
    int ChunkTexAnimSpeed[4];
    int AreaIDColor_Pad2_DrawSelection[4];
    math::vector_4d ChunkXYZBase_Pad1;
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


}
