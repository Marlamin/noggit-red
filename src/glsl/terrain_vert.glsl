// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core

in vec3 position;
in vec3 normal;
in vec3 mccv;
in vec2 texcoord;

layout (std140) uniform matrices
{
  mat4 model_view;
  mat4 projection;
};

out vec3 vary_position;
out vec2 vary_texcoord;
out vec3 vary_normal;
out vec3 vary_mccv;

void main()
{
  gl_Position = projection * model_view * vec4(position, 1.0);
  vary_normal = normal;
  vary_position = position;
  vary_texcoord = texcoord;
  vary_mccv = mccv;
}
