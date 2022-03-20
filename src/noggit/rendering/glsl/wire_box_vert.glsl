#version 330 core

uniform vec3 pointPositions[8];
uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

void main()
{
    gl_Position = projection * model_view * transform * vec4(pointPositions[gl_VertexID], 1.0);
}