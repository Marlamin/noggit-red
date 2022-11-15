#version 330 core

in vec4 position;

uniform mat4 model_view_projection;
uniform vec3 origin;
uniform int height;
uniform float radius;

void main()
{
    vec3 offset = vec3(position.x * radius, position.y, position.z * radius);
    vec3 p = origin + offset;
    gl_Position = model_view_projection * vec4(p,1.);
}