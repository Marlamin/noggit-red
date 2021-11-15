// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>
#include <math/trig.hpp>

namespace math
{
    inline glm::mat4x4 perspective(math::degrees fovy, float aspect, float zNear, float zFar)
    {
        return glm::perspective(math::radians(fovy)._, aspect, zNear, zFar);
    }

    inline glm::mat4x4 ortho(float left, float right, float bottom, float top, float z_near, float z_far)
    {
        return glm::ortho(left,right,bottom, top,z_near,z_far);
    }

    /*
    inline matrix_4x4 perspective(math::degrees fovy, float aspect, float zNear, float zFar)
    {// assuming
    // glm::vec3 lower_left_clipping_plane (left, bottom, -nearVal);
    // glm::vec3 upper_right_clipping_plane (right, top, -nearVal);
    // glm::vec3 eye (0, 0, 0);
    // float clipping_plane_distance (zFar - zNear);

    // with
    float const ymax (zNear * glm::tan(math::radians(fovy)._) / 2.0f);
    // float left (-ymax * aspect);
    // float right (ymax * aspect);
    // float bottom (-ymax);
    // float top (ymax);
    // float nearVal (zNear);
    // float farVal (zFar);

    // multiply matrix by
    // math::matrix_4x4 frustum ( 2 * nearVal / (right - left), 0.0f, (right + left) / (right - left), 0.0f
    //                          , 0.0f, 2 * nearVal / (top - bottom), (top + bottom) / (top - bottom), 0.0f
    //                          , 0.0f, 0.0f, - (farVal + nearVal) / (farVal - nearVal), - 2 * farVal * nearVal / (farVal - nearVal)
    //                          , 0.0f, 0.0f, -1.0f, 0.0f
    //                          );

    // with optimized values
    return { zNear / (ymax * aspect), 0.0f, 0.0f, 0.0f
           , 0.0f, zNear / ymax, 0.0f, 0.0f
           , 0.0f, 0.0f, -(zFar + zNear) / (zFar - zNear), -2 * zFar * zNear / (zFar - zNear)
           , 0.0f, 0.0f, -1.0f, 0.0f
           };
  }

  inline matrix_4x4 ortho(float left, float right, float bottom, float top, float z_near, float z_far)
  {
    float v0 = 2.f / (right - left);
    float v1 = 2.f / (top - bottom);
    float v2 = -2.f / (z_far - z_near);

    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    float tz = -(z_far + z_near) / (z_far - z_near);

    return { v0,  0.f, 0.f, tx
           , 0.f,  v1, 0.f, ty
           , 0.f, 0.f,  v2, tz
           , 0.f, 0.f, 0.f, 1.f
           };
  }*/

  inline matrix_4x4 look_at (glm::vec3 const& eye
                            , glm::vec3 const& center
                            , glm::vec3 const& up
                            )
  {
    glm::vec3 const z = glm::normalize(eye - center);
    glm::vec3 const x = glm::normalize(glm::cross(up, z));
    glm::vec3 const y = glm::normalize(glm::cross(z, x));

    return { x.x, x.y, x.z, glm::dot(x, glm::vec3(-eye.x,-eye.y,-eye.z))
           , y.x, y.y, y.z, glm::dot(y ,glm::vec3(-eye.x,-eye.y,-eye.z))
           , z.x, z.y, z.z, glm::dot(z ,glm::vec3(-eye.x,-eye.y,-eye.z))
           , 0.f, 0.f, 0.f, 1.f
           };
  }
}
