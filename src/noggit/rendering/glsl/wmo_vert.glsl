// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core

in vec4 position;
in vec3 normal;
in vec4 vertex_color;
in vec2 texcoord;
in vec2 texcoord_2;
in uint batch_mapping;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_texcoord;
out vec2 f_texcoord_2;
out vec4 f_vertex_color;

flat out uint flags;
flat out uint shader;
flat out uint tex_array0;
flat out uint tex_array1;
//flat out uint tex_array2;
//flat out uint tex_array3;
flat out uint tex0;
flat out uint tex1;
//flat out uint tex2;
//flat out uint tex3;
flat out uint alpha_test_mode;

layout (std140) uniform matrices
{
  mat4 model_view;
  mat4 projection;
};

uniform mat4 transform;
uniform usamplerBuffer render_batches_tex;

float makeNaN(float nonneg)
{
  return sqrt(-nonneg-1.0);
}

void main()
{
  float NaN = makeNaN(1);

  if (!bool(batch_mapping)) // discard
  {
    gl_Position = vec4(NaN, NaN, NaN, NaN);

    f_position = vec3(0);
    f_normal = vec3(0);
    f_texcoord = vec2(0);
    f_texcoord_2 = vec2(0);
    f_vertex_color = vec4(0);

    flags = 0;
    shader = 0;
    tex_array0 = 0;
    tex_array1 = 0;
    //tex_array2 = 0;
    //tex_array3 = 0;
    tex0 = 0;
    tex1 = 0;
    //tex2 = 0;
    //tex3 = 0;
    alpha_test_mode = 0;
  }
  else
  {
    vec4 pos = transform * position;
    vec4 view_space_pos = model_view * pos;
    gl_Position = projection * view_space_pos;

    f_position = pos.xyz;
    f_normal = mat3(transform) * normal;

    uvec4 batch_first_half = texelFetch(render_batches_tex, int((batch_mapping - 1) * 2));
    uvec4 batch_second_half = texelFetch(render_batches_tex, int((batch_mapping - 1) * 2 + 1));

    // this is just not how it works obviousy, todo
    //uvec4 batch_third_half = texelFetch(render_batches_tex, int((batch_mapping - 1) * 2 + 2));

    flags = batch_first_half.r;
    shader = batch_first_half.g;
    tex_array0 = batch_first_half.b;
    tex_array1 = batch_first_half.a;
    tex0 = batch_second_half.r;
    tex1 = batch_second_half.g;
    alpha_test_mode = batch_second_half.b;

    //tex_array2 = batch_second_half.r;
    //tex_array3 = batch_second_half.g;
    //tex0 = batch_second_half.b;
    //tex1 = batch_second_half.a;
    //tex2 = batch_third_half.r;
    //tex3 = batch_third_half.g;
    //alpha_test_mode = batch_third_half.b;

    // Env and EnvMetal
    if(shader == 3 || shader == 5)
    {
      f_texcoord = texcoord;
      f_texcoord_2 = reflect(normalize(view_space_pos.xyz), f_normal).xy;
    }
    else
    {
      f_texcoord = texcoord;
      f_texcoord_2 = texcoord_2;
    }

    f_vertex_color = vertex_color;
  }
}
