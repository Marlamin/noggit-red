// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <glm/mat4x4.hpp>
#include <optional>
#include <cmath>
#include <iostream>

namespace math
{
  struct sphere;

  struct hit_result
  {
    glm::vec3 position;
    float t = 0;
    bool hit = false;
  };

  struct ray
  {
    ray (glm::vec3 origin, glm::vec3 const& direction)
      : _origin (std::move (origin)), _direction (glm::normalize(direction))
    {
      if (std::isnan(_direction.x) || std::isnan(_direction.y) || std::isnan(_direction.z)) 
      {
        assert(false);
      }
    }

    ray (glm::mat4x4 const& transform, ray const& other)
      : ray (
        glm::vec3(transform * glm::vec4(other._origin, 1.0f)),
        glm::vec3((glm::mat3(transform) * other._direction))
            )
    {}

    std::optional<float> intersect_bounds
      (glm::vec3 const& _min, glm::vec3 const& _max) const noexcept;
    std::optional<float> intersect_triangle
      (glm::vec3 const& _v0, glm::vec3 const& _v1, glm::vec3 const& _v2) const noexcept;

    std::optional<float> intersect_box
    (glm::vec3 const& position, glm::vec3 const& box_min, glm::vec3 const& box_max, glm::vec3 const& rotation) const noexcept;

    hit_result intersects_sphere(sphere const& sphere) const;

    glm::vec3 position (float distance) const
    {
      return _origin + _direction * distance;
    }

  private:
     glm::vec3 _origin;
     glm::vec3 _direction;
  };
}
