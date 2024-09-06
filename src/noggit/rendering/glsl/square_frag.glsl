#version 330 core

uniform vec4 color;

in vec3 f_pos;

out vec4 out_color;

void main()
{
    bool discard_x = mod((f_pos.x + 1.) * 5., 2.) < 1.;
    bool discard_y = mod((f_pos.z + 1.) * 5., 2.) < 1.;
    // discard in a checker board pattern
    if(discard_x != discard_y)
    {
      discard;
    }

    out_color = color;
}