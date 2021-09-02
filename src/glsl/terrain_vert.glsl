// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core

in vec2 position;
in vec2 texcoord;

layout (std140) uniform matrices
{
  mat4 model_view;
  mat4 projection;
};

struct ChunkInstanceData
{
  ivec4 ChunkTextureSamplers;
  ivec4 ChunkTextureArrayIDs;
  ivec4 ChunkHoles_DrawImpass_TexLayerCount_CantPaint;
  ivec4 ChunkTexDoAnim;
  ivec4 ChunkTexAnimSpeed;
  ivec4 AreaIDColor_Pad2_DrawSelection;
  vec4  ChunkXYZBase_Pad1;
  ivec4 ChunkTexAnimDir;
};

layout (std140) uniform chunk_instances
{
  ChunkInstanceData instances[256];
};

uniform sampler2D heightmap;
uniform sampler2D mccv;
uniform int base_instance;
uniform int animtime;
uniform int lod_level;

out vec3 vary_position;
out vec2 vary_texcoord;
out vec2 vary_t0_uv;
out vec2 vary_t1_uv;
out vec2 vary_t2_uv;
out vec2 vary_t3_uv;
out vec3 vary_mccv;
out vec3 vary_normal;
flat out int instanceID;

bool isHoleVertex(uint vertexId, uint hole)
{
  if (hole == 0)
  {
    return false;
  }

  uint blockRow = vertexId / 34;
  uint blockVertexId = vertexId % 34;
  uint shiftedHole = hole >> (blockRow * 4);

  switch(lod_level)
  {
    case 0:
    {
      if ((shiftedHole & 0x1u) != 0)
      {
        if (blockVertexId == 9 || blockVertexId == 10 || blockVertexId == 26 || blockVertexId == 27)
        {
          return true;
        }
      }

      if ((shiftedHole & 0x2u) != 0)
      {
        if (blockVertexId == 11 || blockVertexId == 12 || blockVertexId == 28 || blockVertexId == 29)
        {
          return true;
        }
      }

      if ((shiftedHole & 0x4u) != 0)
      {
        if (blockVertexId == 13 || blockVertexId == 14 || blockVertexId == 30 || blockVertexId == 31)
        {
          return true;
        }
      }

      if ((shiftedHole & 0x8u) != 0)
      {
        if (blockVertexId == 15 || blockVertexId == 16 || blockVertexId == 32 || blockVertexId == 33)
        {
          return true;
        }
      }
      break;
    }
    case 1:
    {
      if ((shiftedHole & 0x1u) != 0)
      {
        if (vertexId == 18 || vertexId == 20 || vertexId == 22 || vertexId == 24)
        {
          return true;
        }
      }

      if ((shiftedHole & 0x2u) != 0)
      {
        if (vertexId == 52 || vertexId == 54 || vertexId == 56 || vertexId == 58)
        {
          return true;
        }
      }

      if ((shiftedHole & 0x4u) != 0)
      {
        if (vertexId == 86 || vertexId == 88 || vertexId == 90 || vertexId == 92)
        {
          return true;
        }
      }

      if ((shiftedHole & 0x8u) != 0)
      {
        if (vertexId == 120 || vertexId == 122 || vertexId == 124 || vertexId == 126)
        {
          return true;
        }
      }
      break;
    }
  }

  return false;
}

float makeNaN(float nonneg)
{
  return sqrt(-nonneg-1.0);
}

vec2 animUVOffset(int do_animate, int spd, int dir)
{
  const float texanimxtab[8] = float[8]( 0, 1, 1, 1, 0, -1, -1, -1 );
  const float texanimytab[8] = float[8]( 1, 1, 0, -1, -1, -1, 0, 1 );
  float fdx = -texanimxtab[dir];
  float fdy = texanimytab[dir];
  int animspd = int(200 * 8.0);
  float f = float((int(animtime*(spd / 7.0f))) % animspd) / float(animspd);

  return vec2(f * fdx, f * fdy);
}

void main()
{
  instanceID = base_instance + gl_InstanceID;
  vec4 normal_pos = texelFetch(heightmap, ivec2(gl_VertexID, instanceID), 0);
  vec3 pos_base = instances[instanceID].ChunkXYZBase_Pad1.xyz;
  vec3 pos = vec3(position.x + pos_base.x, pos_base.y + normal_pos.a, position.y + pos_base.z);

  bool is_hole = isHoleVertex(gl_VertexID, instances[instanceID].ChunkHoles_DrawImpass_TexLayerCount_CantPaint.r);

  float NaN = makeNaN(1);

  gl_Position = projection * model_view * (is_hole ? vec4(NaN, NaN, NaN, 1.0) : vec4(pos, 1.0));

  vary_normal = normal_pos.rgb;
  vary_position = pos;
  vary_mccv = texelFetch(mccv, ivec2(gl_VertexID, instanceID), 0).rgb;

  vary_t0_uv = texcoord + animUVOffset(instances[instanceID].ChunkTexDoAnim.r,
    instances[instanceID].ChunkTexAnimSpeed.r, instances[instanceID].ChunkTexAnimDir.r);

  vary_t1_uv = texcoord + animUVOffset(instances[instanceID].ChunkTexDoAnim.g,
    instances[instanceID].ChunkTexAnimSpeed.g, instances[instanceID].ChunkTexAnimDir.g);

  vary_t2_uv = texcoord + animUVOffset(instances[instanceID].ChunkTexDoAnim.b,
    instances[instanceID].ChunkTexAnimSpeed.b, instances[instanceID].ChunkTexAnimDir.b);

  vary_t3_uv = texcoord + animUVOffset(instances[instanceID].ChunkTexDoAnim.a,
    instances[instanceID].ChunkTexAnimSpeed.a, instances[instanceID].ChunkTexAnimDir.a);

  vary_texcoord = texcoord;

}
