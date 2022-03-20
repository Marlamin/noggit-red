// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#version 410 core

layout (std140) uniform matrices
{
    mat4 model_view;
    mat4 projection;
};

uniform vec3 aabb[2];

void main()
{
    vec4 pos;
    vec3 aabb_min = aabb[0];
    vec3 aabb_max = aabb[1];

    switch (gl_VertexID)
    {
        case 0: // A
        {
            pos = vec4(aabb_min.x, aabb_max.y, aabb_min.z, 1.0);
            break;
        }
        case 1: // B
        {
            pos = vec4(aabb_min.x, aabb_max.y, aabb_max.z, 1.0);
            break;
        }
        case 2: // C
        {
            pos = vec4(aabb_max.x, aabb_max.y, aabb_min.z, 1.0);
            break;
        }
        case 3: // D
        {
            pos = vec4(aabb_max.x, aabb_max.y, aabb_max.z, 1.0);
            break;
        }
        case 4: // E
        {
            pos = vec4(aabb_min.x, aabb_min.y, aabb_min.z, 1.0);
            break;
        }
        case 5: // F
        {
            pos = vec4(aabb_min.x, aabb_min.y, aabb_max.z, 1.0);
            break;
        }
        case 6: // G
        {
            pos = vec4(aabb_max.x, aabb_min.y, aabb_min.z, 1.0);
            break;
        }
        case 7: // H
        {
            pos = vec4(aabb_max.x, aabb_min.y, aabb_max.z, 1.0);
            break;
        }

    }

    gl_Position = projection * model_view * pos;
}
