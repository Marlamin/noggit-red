// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <string>
#include <ostream>
#include <vector>
#include <cmath>

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

  // inside of axis aligned box
  inline bool is_inside_of_aabb(const glm::vec3& pos, const glm::vec3& a, const glm::vec3& b)
  {
      return a.x < pos.x&& b.x > pos.x
          && a.y < pos.y&& b.y > pos.y
          && a.z < pos.z&& b.z > pos.z;
  }

  // if a point is in a rectangle in 2D space
  inline bool is_inside_of_aabb_2d(const glm::vec2& pos, const glm::vec2& a, const glm::vec2& b)
  {
    return a.x < pos.x && b.x > pos.x
      && a.y < pos.y && b.y > pos.y;
  }

  // check if a rectangle intersects with another in 2D space
  inline bool boxIntersects(const glm::vec2& minA, const glm::vec2& maxA, const glm::vec2& minB, const glm::vec2& maxB)
  {
    // Check if one box is to the left or right of the other
    if (minA.x > maxB.x || minB.x > maxA.x)
      return false;

    // Check if one box is above or below the other
    if (minA.y > maxB.y || minB.y > maxA.y)
      return false;

    // The boxes intersect
    return true;

    // Note : if you need to use the inetrsection area instead use : 
    // glm::vec2 intersectionMin = glm::max(box_a[0], box_b[0]);
    // glm::vec2 intersectionMax = glm::min(box_a[1], box_b[1]);
    // 
    // if (!(intersectionMin.x < intersectionMax.x) || !(intersectionMin.y < intersectionMax.y))
    //   continue;
    // example using center of intersection area
    // glm::vec2 intersectionCenter = (intersectionMin + intersectionMax) * 0.5f;
  }

  bool is_inside_of_polygon(const glm::vec2& pos, const std::vector<glm::vec2>& polygon);

  // bool is_inside_of_polygon(const glm::vec2& pos, const std::vector<glm::vec2>& polygon) {
  //   int n = polygon.size();
  //   bool inside = false;
  // 
  //   // Iterate through each edge of the polygon
  //   for (int i = 0; i < n; ++i) {
  //     // Get the current vertex and the next vertex (looping back to the start)
  //     glm::vec2 v1 = polygon[i];
  //     glm::vec2 v2 = polygon[(i + 1) % n];
  // 
  //     // Check if the point is within the y-bounds of the edge
  //     if ((v1.y > pos.y) != (v2.y > pos.y)) {
  //       // Compute the x-coordinate of the intersection point
  //       float intersectX = (pos.y - v1.y) * (v2.x - v1.x) / (v2.y - v1.y) + v1.x;
  // 
  //       // Check if the intersection point is to the right of the point
  //       if (pos.x < intersectX) {
  //         inside = !inside;
  //       }
  //     }
  //   }
  // 
  //   return inside;
  // }
}
