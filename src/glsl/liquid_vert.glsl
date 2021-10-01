// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core

const float TILESIZE = 533.33333f;
const float CHUNKSIZE = ((TILESIZE) / 16.0f);
const float UNITSIZE = (CHUNKSIZE / 8.0f);

in vec2 position;

uniform vec3 camera;

struct LiquidChunkInstanceDataUniformBlock
{
  uint texture_array;
  uint type;
  float xbase;
  float zbase;
  float anim_u;
  float anim_v;
  uint subchunks_1;
  uint subchunks_2;
  uint n_tex_frames;
  uint _pad1;
  uint _pad2;
  uint _pad3;
  uint _pad4;
  uint _pad5;
  uint _pad6;
  uint _pad7;
};

layout (std140) uniform matrices
{
  mat4 model_view;
  mat4 projection;
};

layout (std140) uniform liquid_layers_params
{
  LiquidChunkInstanceDataUniformBlock[256] layer_params;
};

uniform sampler2DArray vertex_data;

uniform mat4 transform;

uniform int use_transform = int(0);

uniform float animtime;

out float depth_;
out vec2 tex_coord_;
out float dist_from_camera_;
flat out uint tex_array;
flat out uint type;
flat out vec2 anim_uv;
flat out int tex_frame;

bool hasSubchunk(uint x, uint z, uint subchunks_first, uint subchunks_second)
{
  return bool(((subchunks_second * uint(z >= 4) + subchunks_first * uint(z < 4)) >> (z * 8 + x - 32 * uint(z >= 4))) & 1u);
}

float makeNaN(float nonneg)
{
  return sqrt(-nonneg-1.0);
}

int get_texture_frame(int n_frames)
{
  return int(ceil(animtime / 60)) % n_frames;
}

void main()
{
  uint vertex_x = uint(position.x / UNITSIZE);
  uint vertex_y = uint(position.y / UNITSIZE);

  vec4 v_data = texelFetch(vertex_data, ivec3(vertex_x, vertex_y, gl_InstanceID), 0);

  LiquidChunkInstanceDataUniformBlock params = layer_params[gl_InstanceID];

  uint sschunk = gl_VertexID / 3 / 2;
  uint schunk_x = sschunk % 8;
  uint schunk_z = sschunk / 8;

  vec4 final_pos;

  if (hasSubchunk(schunk_x, schunk_z, params.subchunks_1, params.subchunks_2))
  {
    final_pos = vec4(position.x + params.xbase, v_data.r, position.y + params.zbase, 1.0);
  }
  else
  {
    float NaN = makeNaN(1);
    final_pos = vec4(NaN, NaN, NaN, NaN);
  }

  depth_ = v_data.g;
  tex_coord_ = v_data.ba;
  dist_from_camera_ = distance(camera, final_pos.xyz);
  tex_array = params.texture_array;
  type = params.type;
  tex_frame = get_texture_frame(int(params.n_tex_frames));
  anim_uv = vec2(params.anim_u, params.anim_v);

  if(use_transform == 1)
  {
    gl_Position = projection * model_view * transform * final_pos;
  }
  else
  {
    gl_Position = projection * model_view * final_pos;
  }
}
