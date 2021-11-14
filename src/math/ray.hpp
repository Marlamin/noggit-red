// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>
#include <math/vector_3d.hpp>

#include <boost/optional/optional.hpp>

namespace math
{
  struct ray
  {
    ray (glm::vec3 origin, glm::vec3 const& direction): _origin (std::move (origin)), _direction (glm::normalize(direction))
    {}

    ray (matrix_4x4 const& transform, ray const& other): ray (
        glm::vec3(
            (transform * glm::vec4(other._origin.x, other._origin.y, other._origin.z, 1.0))),
        glm::vec3((transform * glm::vec4(other._direction.x, other._direction.y, other._direction.z, 0.0)))
            )
    {}

    boost::optional<float> intersect_bounds
      (glm::vec3 const& _min, glm::vec3 const& _max) const;
    boost::optional<float> intersect_triangle
      (glm::vec3 const& _v0, glm::vec3 const& _v1, glm::vec3 const& _v2) const;

    glm::vec3 position (float distance) const
    {
      return _origin + _direction * distance;
    }

  private:
     glm::vec3 _origin;
     glm::vec3 _direction;
  };
}
