// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 330 core

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform bool use_vertex_color;

uniform bool draw_fog;
uniform bool unfogged;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
uniform vec3 camera;

uniform bool unlit;
uniform bool exterior_lit;
uniform vec3 exterior_light_dir;
uniform vec3 exterior_diffuse_color;
uniform vec3 exterior_ambient_color;
uniform vec3 ambient_color;

uniform float alpha_test;

uniform int shader_id;

in vec3 f_position;
in vec3 f_normal;
in vec2 f_texcoord;
in vec2 f_texcoord_2;
in vec4 f_vertex_color;

out vec4 out_color;

vec3 lighting(vec3 material)
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
    ambient_term = exterior_ambient_color;
    diffuse_term = exterior_diffuse_color;
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
    float nDotL = clamp(dot(normalize(f_normal), -normalize(exterior_light_dir)), 0.0, 1.0);

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
  bool fog = draw_fog && !unfogged;

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
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 5) // EnvMetal
  {
    vec3 env = tex_2.rgb * tex.rgb * tex.a;
    out_color = vec4(lighting(tex.rgb) + env, 1.);
  }
  else if(shader_id == 6) // TwoLayerDiffuse
  {
    vec3 layer2 = mix(tex.rgb, tex_2.rgb, tex_2.a);
    out_color = vec4(lighting(mix(layer2, tex.rgb, vertex_color.a)), 1.);
  }
  else // default shader, used for shader_id 0,1,2,4 (Diffuse, Specular, Metal, Opaque)
  {
    out_color = vec4(lighting(tex.rgb), 1.);
  }

  if(fog)
  {
    float start = fog_end * fog_start;

    vec3 fogParams;
    fogParams.x = -(1.0 / (fog_end - start));
    fogParams.y = (1.0 / (fog_end - start)) * fog_end;
    fogParams.z = 1.0;

    float f1 = (dist_from_camera * fogParams.x) + fogParams.y;
    float f2 = max(f1, 0.0);
    float f3 = pow(f2, fogParams.z);
    float f4 = min(f3, 1.0);

    float fogFactor = 1.0 - f4;

    out_color.rgb = mix(out_color.rgb, fog_color.rgb, fogFactor);
  }

  if(out_color.a < alpha_test)
  {
    discard;
  }
}
