// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>

#include <array>
#include <external/glm/glm.hpp>

namespace math
{
  frustum::frustum (glm::mat4x4 const& matrix)
  {
    const glm::vec4 column_0 = matrix[0];
    const glm::vec4 column_1 = matrix[1];
    const glm::vec4 column_2 = matrix[2];
    const glm::vec4 column_3 = matrix[3];

    _planes[RIGHT] = column_3 - column_0;
    _planes[LEFT] = column_3 + column_0;
    _planes[TOP] = column_3 - column_1;
    _planes[BOTTOM] = column_3 + column_1;
    _planes[BACK] = column_3 - column_2;
    _planes[FRONT] = column_3 + column_2;
  }

  bool frustum::contains (const glm::vec3& point) const
  {
    for (auto const& plane : _planes)
    {
      if (glm::dot(plane.normal() , point) <= -plane.distance())
      {
        return false;
      }
    }
    return true;
  }

  bool frustum::intersects (const std::array<glm::vec3, 8>& intersect_points) const
  {
    for (auto const& plane : _planes)
    {
      for (auto const& point : intersect_points)
      {
        if (glm::dot(plane.normal(), point) > -plane.distance())
        {
          //! \note C does not know how to continue out of two loops otherwise.
          goto intersects_next_side;
        }
      }

      return false;

    intersects_next_side:;
    }

    return true;

  }

  bool frustum::intersects ( const glm::vec3& v1
                           , const glm::vec3& v2
                           ) const
  {
    for (auto const& plane : _planes)
    {

      glm::vec3 vmin;
      glm::vec3 normal = plane.normal();

      // X axis 
      if (normal.x > 0)
      {
        vmin.x = v1.x;
      }
      else
      {
        vmin.x = v2.x;
      }

      // Y axis 
      if (normal.y > 0)
      {
        vmin.y = v1.y;
      }
      else
      {
        vmin.y = v2.y;
      }

      // Z axis 
      if (normal.z > 0)
      {
        vmin.z = v1.z;
      }
      else
      {
        vmin.z = v2.z;
      }

      auto plane_normal = normal;

      if (glm::dot(plane_normal, vmin) + plane.distance() <= 0)
      {
        return false;
      }

    }

    return true;
  }


  bool frustum::intersectsSphere ( const glm::vec3& position
                                 , const float& radius
                                 ) const
  {
    for (auto const& plane : _planes)
    {
      const float distance = glm::dot(plane.normal(), position+ plane.distance());
      if (distance < -radius)
      {
        return false;
      }
      else if (std::abs (distance) < radius)
      {
        return true;
      }
    }
    return true;
  }
}
