// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/scripting_tool.hpp>

#include <sol/sol.hpp>
#include <cmath>

namespace Noggit
{
  namespace Scripting
  {
    int round(float a1) { return std::floor(a1); }
    float pow(float a1, float a2) { return ::pow(a1, a2); }
    float log10(float arg) { return ::log10(arg); }
    float log(float arg) { return ::log(arg); }
    int ceil(float arg) { return ::ceil(arg); }
    int floor(float arg) { return std::floor(arg); }
    float exp(float arg) { return ::exp(arg); }
    float cbrt(float arg) { return ::cbrt(arg); }
    float acosh(float arg) { return ::acosh(arg); }
    float asinh(float arg) { return ::asinh(arg); }
    float atanh(float arg) { return ::atanh(arg); }
    float cosh(float arg) { return ::cosh(arg); }
    float sinh(float arg) { return ::sinh(arg); }
    float tanh(float arg) { return ::tanh(arg); }
    float acos(float arg) { return ::acos(arg); }
    float asin(float arg) { return ::asin(arg); }
    float atan(float arg) { return ::atan(arg); }
    float cos(float arg) { return ::cos(arg); }
    float sin(float arg) { return ::sin(arg); }
    float tan(float arg) { return ::tan(arg); }
    float sqrt(float arg) { return ::sqrt(arg); }
    float abs(float arg) { return ::abs(arg); }
    float lerp(float from, float to, float ratio) { return from + ratio * (to - from); }
    float dist_2d(glm::vec3 const& from, glm::vec3 const& to)
    {
      return std::sqrt(std::pow(from.x - to.x, 2) + std::pow(from.z - to.z, 2));
    }

    int dist_2d_compare(glm::vec3 const& from, glm::vec3 const& to, float compare)
    {
      float dist = std::pow(from.x - to.x, 2) + std::pow(from.z - to.z, 2);
      compare = std::pow(compare, 2);
      return dist > compare ? 1 : dist == compare ? 0 : -1;
    }

    glm::vec3 rotate_2d(glm::vec3 const& point, glm::vec3 const& origin, float angle)
    {
      float s = std::sin(angle * 0.0174532925);
      float c = std::cos(angle * 0.0174532925);

      float lx = point.x - origin.x;
      float lz = point.z - origin.z;

      float nx = lx * c - lz * s + origin.x;
      float nz = lx * s + lz * c + origin.z;

      return glm::vec3(nx, point.y, nz);
    }

    void register_math(script_context * state)
    {
      state->set_function("round",round);
      state->set_function("pow",pow);
      state->set_function("log10",log10);
      state->set_function("log",log);
      state->set_function("ceil",ceil);
      state->set_function("floor",floor);
      state->set_function("exp",exp);
      state->set_function("cbrt",cbrt);
      state->set_function("acosh",acosh);
      state->set_function("asinh",asinh);
      state->set_function("atanh",atanh);
      state->set_function("cosh",cosh);
      state->set_function("sinh",sinh);
      state->set_function("tanh",tanh);
      state->set_function("acos",acos);
      state->set_function("asin",asin);
      state->set_function("atan",atan);
      state->set_function("cos",cos);
      state->set_function("sin",sin);
      state->set_function("tan",tan);
      state->set_function("sqrt",sqrt);
      state->set_function("abs",abs);
      state->set_function("lerp",lerp);
      state->set_function("dist_2d",dist_2d);
      state->set_function("dist_2d_compare",dist_2d_compare);
      state->set_function("rotate_2d",rotate_2d);

      state->new_usertype<glm::vec3>("vector_3d"
        , "x", &glm::vec3::x
        , "y", &glm::vec3::y
        , "z", &glm::vec3::z
      );
    }
  } // namespace Scripting
} // namespace Noggit
