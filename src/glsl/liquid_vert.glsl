// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

in vec4 position;
in vec2 tex_coord;
in float depth;

uniform vec3 camera;

uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

uniform int use_transform = int(0);

out float depth_;
out vec2 tex_coord_;
out float dist_from_camera_;

void main()
{
  depth_ = depth;
  tex_coord_ = tex_coord;
  dist_from_camera_ = distance(camera, position.xyz);

  if(use_transform == 1)
  {
    gl_Position = projection * model_view * transform * position;
  }
  else
  {
    gl_Position = projection * model_view * position;
  }
}
