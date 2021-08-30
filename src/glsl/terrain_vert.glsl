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
  vec4  ChunkTexAnim_0_1;
  vec4  ChunkTexAnim_2_3;
  vec4  AreaIDColor_DrawSelection;
  vec4  ChunkXYZBase_Pad1;
  vec4 pad2;
};

layout (std140) uniform chunk_instances
{
  ChunkInstanceData instances[256];
};

uniform sampler2D heightmap;
uniform sampler2D mccv;

out vec3 vary_position;
out vec2 vary_texcoord;
out vec3 vary_normal;
out vec3 vary_mccv;
flat out int instanceID;

void main()
{
  vec4 normal_pos = texelFetch(heightmap, ivec2(gl_VertexID, gl_InstanceID), 0);
  vec3 pos_base = instances[gl_InstanceID].ChunkXYZBase_Pad1.xyz;
  vec3 pos = vec3(position.x + pos_base.x, pos_base.y + normal_pos.a, position.y + pos_base.z);

  gl_Position = projection * model_view * vec4(pos, 1.0);
  vary_normal = normal_pos.rgb;
  vary_position = pos;
  vary_texcoord = texcoord;
  vary_mccv = texelFetch(mccv, ivec2(gl_VertexID, gl_InstanceID), 0).rgb;
  instanceID = gl_InstanceID;
}
