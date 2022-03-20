// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 pos;
in vec3 normal;
in vec2 texcoord1;
in vec2 texcoord2;
in uvec4 bones_weight;
in uvec4 bones_indices;

#ifdef instanced
  in mat4 transform;
#else
  uniform mat4 transform;
#endif

uniform samplerBuffer bone_matrices;

out vec2 uv1;
out vec2 uv2;
out float camera_dist;
out vec3 norm;

layout (std140) uniform matrices
{
  mat4 model_view;
  mat4 projection;
};

uniform int tex_unit_lookup_1;
uniform int tex_unit_lookup_2;

uniform mat4 tex_matrix_1;
uniform mat4 tex_matrix_2;

uniform bool anim_bones;

// code from https://wowdev.wiki/M2/.skin#Environment_mapping
vec2 sphere_map(vec3 vert, vec3 norm)
{
  vec3 normPos = -(normalize(vert));
  vec3 temp = (normPos - (norm * (2.0 * dot(normPos, norm))));
  temp = vec3(temp.x, temp.y, temp.z + 1.0);
 
  return ((normalize(temp).xy * 0.5) + vec2(0.5));
}

vec2 get_texture_uv(int tex_unit_lookup, vec3 vert, vec3 norm)
{
  if(tex_unit_lookup == 0)
  {
    return sphere_map(vert, norm);
  }
  else if(tex_unit_lookup == 1)
  {
    return (transpose(tex_matrix_1) * vec4(texcoord1, 0.0, 1.0)).xy;
  }
  else if(tex_unit_lookup == 2)
  {
    return (transpose(tex_matrix_2) * vec4(texcoord2, 0.0, 1.0)).xy;
  }
  else
  {
    return vec2(0.0);
  }
}

mat4 get_bone_matrix(uint bone_index)
{
  mat4 matrix;
  int pixel_start = int(bone_index) * 4;
  matrix[0] = texelFetch(bone_matrices, pixel_start).rgba;
  matrix[1] = texelFetch(bone_matrices, pixel_start + 1).rgba;
  matrix[2] = texelFetch(bone_matrices, pixel_start + 2).rgba;
  matrix[3] = texelFetch(bone_matrices, pixel_start + 3).rgba;

  return matrix;
}

void main()
{
  mat4 boneTransformMat = mat4(0);

  if (anim_bones)
  {
    boneTransformMat += (float(bones_weight.x) / 255.0) * get_bone_matrix(bones_indices.x);
    boneTransformMat += (float(bones_weight.y) / 255.0) * get_bone_matrix(bones_indices.y);
    boneTransformMat += (float(bones_weight.z) / 255.0) * get_bone_matrix(bones_indices.z);
    boneTransformMat += (float(bones_weight.w) / 255.0) * get_bone_matrix(bones_indices.w);
  }
  else
  {
    boneTransformMat = mat4(1);
  }

  mat4 cameraMatrix = model_view * transform * boneTransformMat;
  mat3 normMatrix = mat3(transform * boneTransformMat);

  vec4 vertex = cameraMatrix * pos;

  // important to normalize because of the scaling !!
  norm = normalize(normMatrix * normal);

  uv1 = get_texture_uv(tex_unit_lookup_1, vertex.xyz, norm);
  uv2 = get_texture_uv(tex_unit_lookup_2, vertex.xyz, norm);

  camera_dist = -vertex.z;
  gl_Position = projection * vertex;
}
