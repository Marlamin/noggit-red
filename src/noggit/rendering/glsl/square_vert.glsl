#version 330 core

in vec4 position;

uniform mat4 model_view_projection;
uniform vec3 origin;
uniform float radius;
uniform float inclination;
uniform float orientation;

void main()
{
    vec4 pos = position;
    float cos_o = cos(orientation);
    float sin_o = sin(orientation);

    pos.y += pos.x * tan(inclination) * radius;

    pos.x = (position.x*cos_o - position.z * sin_o) * radius;
    pos.z = (position.z*cos_o + position.x * sin_o) * radius;

    pos.xyz += origin;
    gl_Position = model_view_projection * pos;
}