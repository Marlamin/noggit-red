#version 330 core

in vec4 position;

uniform mat4 model_view_projection;
uniform vec3 origin;
uniform float radius;

void main()
{
    vec4 pos = position;
    pos.xyz *= radius;

    vec3 origin_fixed = vec3(origin.x - radius / 2.0f, origin.y, origin.z - radius / 2.0f);
    pos.xyz += origin_fixed;
    gl_Position = model_view_projection * pos;
}