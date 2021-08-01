// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

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

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform bool use_vertex_color;

uniform bool unfogged;
uniform vec3 camera;

uniform bool unlit;
uniform bool exterior_lit;
uniform vec3 ambient_color;

uniform float alpha_test;

uniform int shader_id;

in vec3 f_position;
in vec3 f_normal;
in vec2 f_texcoord;
in vec2 f_texcoord_2;
in vec4 f_vertex_color;

out vec4 out_color;

vec3 apply_lighting(vec3 material)
{
  vec3 ambient_term;
  vec3 diffuse_term;
  vec3 vertex_color = use_vertex_color ? f_vertex_color.rgb : vec3(0.);

  if(unlit)
  {
    ambient_term = vec3(0.0);
    diffuse_term = vec3(0.0);
  }
  else if(exterior_lit)
  {
    ambient_term = AmbientColor_FogEnd.xyz;
    diffuse_term = DiffuseColor_FogStart.xyz;
  }
  else
  {
    ambient_term = ambient_color;
    diffuse_term = vec3(0.0);
  }

  // apply world lighting
  vec3 currColor;
  vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
  vec3 accumlatedLight = vec3(1.0, 1.0, 1.0);

  if(!unlit)
  {
    float nDotL = clamp(dot(normalize(f_normal), -normalize(vec3(-LightDir_FogRate.x, LightDir_FogRate.z, -LightDir_FogRate.y))), 0.0, 1.0);

    vec3 ambientColor = ambient_term + vertex_color;

    vec3 skyColor = (ambientColor * 1.10000002);
    vec3 groundColor = (ambientColor * 0.699999988);

    currColor = mix(groundColor, skyColor, 0.5 + (0.5 * nDotL));
    lDiffuse = diffuse_term * nDotL;
  }
  else
  {
    currColor = ambient_color + vertex_color;
    accumlatedLight = vec3(0.0f, 0.0f, 0.0f);
  }

  return clamp(material.rgb * (currColor + lDiffuse), 0.0, 1.0);
}

void main()
{
  float dist_from_camera = distance(camera, f_position);
  bool fog = FogColor_FogOn.w != 0 && !unfogged;

  vec4 tex = texture(tex1, f_texcoord);
  vec4 tex_2 = texture(tex2, f_texcoord_2);

  if(tex.a < alpha_test)
  {
    discard;
  }

  vec4 vertex_color = vec4(0., 0., 0., 1.f);
  vec3 light_color = vec3(1.);

  if(use_vertex_color) 
  {
    vertex_color = f_vertex_color;
  }


  // see: https://github.com/Deamon87/WebWowViewerCpp/blob/master/wowViewerLib/src/glsl/wmoShader.glsl
  if(shader_id == 3) // Env
  {
    vec3 env = tex_2.rgb * tex.rgb;
    out_color = vec4(apply_lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 5) // EnvMetal
  {
    vec3 env = tex_2.rgb * tex.rgb * tex.a;
    out_color = vec4(apply_lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 6) // TwoLayerDiffuse
  {
    vec3 layer2 = mix(tex.rgb, tex_2.rgb, tex_2.a);
    out_color = vec4(apply_lighting(mix(layer2, tex.rgb, vertex_color.a)), 1.);
  }
  else // default shader, used for shader_id 0,1,2,4 (Diffuse, Specular, Metal, Opaque)
  {
    out_color = vec4(apply_lighting(tex.rgb), 1.);
  }

  if(fog)
  {
    float start = AmbientColor_FogEnd.w * DiffuseColor_FogStart.w;

    vec3 fogParams;
    fogParams.x = -(1.0 / (AmbientColor_FogEnd.w - start));
    fogParams.y = (1.0 / (AmbientColor_FogEnd.w - start)) * AmbientColor_FogEnd.w;
    fogParams.z = LightDir_FogRate.w;

    float f1 = (dist_from_camera * fogParams.x) + fogParams.y;
    float f2 = max(f1, 0.0);
    float f3 = pow(f2, fogParams.z);
    float f4 = min(f3, 1.0);

    float fogFactor = 1.0 - f4;

    out_color.rgb = mix(out_color.rgb, FogColor_FogOn.rgb, fogFactor);
  }

  if(out_color.a < alpha_test)
  {
    discard;
  }
}
