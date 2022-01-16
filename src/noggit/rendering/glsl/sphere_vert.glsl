#version 330 core

in vec4 position;

uniform mat4 model_view_projection;
uniform vec3 origin;
uniform float radius;

void main()
{
    vec4 pos = position;
    pos.xyz *= radius;
    pos.xyz += origin;
    gl_Position = model_view_projection * pos;
}