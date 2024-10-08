// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <memory>
#include <string>
#include <glm/vec3.hpp>

namespace Noggit
{
  namespace Scripting
  {
    class scripting_tool;
    class script_context;
    int round(float a1);
    float pow(float a1, float a2);
    float log10(float arg);
    float log(float arg);
    int ceil(float arg);
    int floor(float arg);
    float exp(float arg);
    float cbrt(float arg);
    float acosh(float arg);
    float asinh(float arg);
    float atanh(float arg);
    float cosh(float arg);
    float sinh(float arg);
    float tanh(float arg);
    float acos(float arg);
    float asin(float arg);
    float atan(float arg);
    float cos(float arg);
    float sin(float arg);
    float tan(float arg);
    float sqrt(float arg);
    float abs(float arg);
    float lerp(float from, float to, float amount);
    float dist_2d(glm::vec3 const& from, glm::vec3 const& to);
    int dist_2d_compare(glm::vec3 const& from, glm::vec3 const& to, float dist);
    glm::vec3 rotate_2d(glm::vec3 const& point, glm::vec3 const& origin, float angleDeg);

    void register_math(script_context * state);
  } // namespace Scripting
} // namespace Noggit
