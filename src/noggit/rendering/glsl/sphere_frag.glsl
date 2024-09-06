#version 330 core

uniform vec4 color;

out vec4 out_color;

void main()
{
    if(gl_FragCoord.x < 0.5)
    {
        discard;
    }
    out_color = color;
}