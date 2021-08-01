// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D tex;

layout (std140) uniform lighting
{
  vec4 DiffuseColor_FogStart;
  vec4 AmbientColor_FogEnd;
  vec4 FogColor_FogOn;
  vec4 LightDir_FogRate;
  vec4 OceanColorLight;
  vec4 OceanColorDark;
  vec4 RiverColorLight;
  vec4 RiverColorDark;
};


uniform int type;
uniform float animtime;
uniform vec2 param;

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
              ? mix (OceanColorLight, OceanColorDark, depth_) 
              : mix (RiverColorLight, RiverColorDark, depth_)
              ;
              
    //clamp shouldn't be needed
    out_color = vec4 (clamp(texel + lerp, 0.0, 1.0).rgb, lerp.a);
  }

  if (FogColor_FogOn.w != 0)
  {
    float start = AmbientColor_FogEnd.w * DiffuseColor_FogStart.w;

    vec3 fogParams;
    fogParams.x = -(1.0 / (AmbientColor_FogEnd.w - start));
    fogParams.y = (1.0 / (AmbientColor_FogEnd.w - start)) * AmbientColor_FogEnd.w;
    fogParams.z = LightDir_FogRate.w;

    float f1 = (dist_from_camera_ * fogParams.x) + fogParams.y;
    float f2 = max(f1, 0.0);
    float f3 = pow(f2, fogParams.z);
    float f4 = min(f3, 1.0);

    float fogFactor = 1.0 - f4;

    out_color.rgb = mix(out_color.rgb, FogColor_FogOn.rgb, fogFactor);
  }
}
