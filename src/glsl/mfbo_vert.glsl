// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 position;

layout (std140) uniform matrices
{
  mat4 model_view;
  mat4 projection;
};

void main()
{
  gl_Position = (model_view * projection) * position;
}
