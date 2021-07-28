// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D tex;
uniform vec4 ocean_color_light;
uniform vec4 ocean_color_dark;
uniform vec4 river_color_light;
uniform vec4 river_color_dark;

uniform vec3 fog_color;
uniform float fog_start;
uniform float fog_end;

uniform int type;
uniform float animtime;
uniform vec2 param;
uniform bool draw_fog;

in float depth_;
in vec2 tex_coord_;
in float dist_from_camera_;

out vec4 out_color;

vec2 rot2(vec2 p, float degree)
{
  float a = radians(degree);
  return mat2(cos(a), -sin(a), sin(a), cos(a))*p;
}

void main()
{
  // lava || slime
  if(type == 2 || type == 3)
  {
    out_color = texture(tex, tex_coord_ + vec2(param.x*animtime, param.y*animtime));
  }
  else
  {
    vec2 uv = rot2(tex_coord_ * param.x, param.y);
    vec4 texel = texture(tex, uv);
    vec4 lerp = (type == 1)
              ? mix (ocean_color_light, ocean_color_dark, depth_) 
              : mix (river_color_light, river_color_dark, depth_)
              ;
              
    //clamp shouldn't be needed
    out_color = vec4 (clamp(texel + lerp, 0.0, 1.0).rgb, lerp.a);
  }

  if (draw_fog)
  {
    float start = fog_end * fog_start;

    vec3 fogParams;
    fogParams.x = -(1.0 / (fog_end - start));
    fogParams.y = (1.0 / (fog_end - start)) * fog_end;
    fogParams.z = 1.0;

    float f1 = (dist_from_camera_ * fogParams.x) + fogParams.y;
    float f2 = max(f1, 0.0);
    float f3 = pow(f2, fogParams.z);
    float f4 = min(f3, 1.0);

    float fogFactor = 1.0 - f4;

    out_color.rgb = mix(out_color.rgb, fog_color.rgb, fogFactor);
  }
}
