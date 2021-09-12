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
    CHUNK_INSTANCE_DATA
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
    int draw_impass_overlay;
    int draw_paintability_overlay;
    int draw_selection_overlay;
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
    unsigned TextureArray_Pad3[4];
    float ChunkXY_Animation[4];
};

}
