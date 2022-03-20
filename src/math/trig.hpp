// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <string>
#include <iostream>
#include <cmath>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace math
{
  namespace
  {
      inline float normalize_degrees(float deg)
      {
        return deg - std::floor((deg + 180.f) / 360.f) * 360.f;
      }
  }

  struct radians;

  struct degrees
  {
    explicit degrees (float x) : _ (normalize_degrees(x)) {}
    degrees (radians);

    float _;

    inline degrees operator+ (const degrees &v) const
    {
      return degrees (_ + v._);
    }

    inline degrees operator- (const degrees &v) const
    {
      return degrees (_ - v._);
    }

    inline degrees operator-() const
    {
      return degrees (-_);
    }

    inline degrees& operator+= (const degrees &v)
    {
      return *this = *this + v;
    }

    inline degrees& operator-= (const degrees &v)
    {
      return *this = *this - v;
    }

    friend std::ostream& operator<< (std::ostream& os, degrees const& v)
    {
      return os << std::to_string(v._) << std::string("Degrees");
    }

      using vec3 = glm::vec3;
  };

  struct radians
  {
    explicit radians (float x) : _ (x) {}
    radians (degrees);

    float _;

    using vec3 = glm::vec3;
  };

  inline degrees::degrees (radians x) : _ (x._ * 180.0f / glm::pi<float>()) {}
  inline radians::radians (degrees x) : _ (x._ * glm::pi<float>() / 180.0f) {}

  inline void rotate(float x0, float y0, float* x, float* y, radians angle)
  {
      const float xa(*x - x0);
      const float ya(*y - y0);
      *x = xa * glm::cos(angle._) - ya * glm::sin(angle._) + x0;
      *y = xa * glm::sin(angle._) + ya * glm::cos(angle._) + y0;
  }

  inline bool is_inside_of(const glm::vec3& pos, const glm::vec3& a, const glm::vec3& b)
  {
      return a.x < pos.x&& b.x > pos.x
          && a.y < pos.y&& b.y > pos.y
          && a.z < pos.z&& b.z > pos.z;
  }
}
