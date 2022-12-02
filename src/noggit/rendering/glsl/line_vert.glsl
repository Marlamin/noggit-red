#version 330 core

in vec4 position;

uniform mat4 model_view_projection;
uniform vec3 origin;

void main()
{
    vec4 pos = position;
    pos.xyz += origin;
    gl_Position = model_view_projection * pos;
}