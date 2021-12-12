// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/ray.hpp>

#include <limits>

namespace math
{
  std::optional<float> ray::intersect_bounds
    (glm::vec3 const& min, glm::vec3 const& max) const
  {
    float tmin (std::numeric_limits<float>::lowest());
    float tmax (std::numeric_limits<float>::max());

    if (_direction.x != 0.0f)
    {
      float const tx1 ((min.x - _origin.x) / _direction.x);
      float const tx2 ((max.x - _origin.x) / _direction.x);

      tmin = std::max (tmin, std::min (tx1, tx2));
      tmax = std::min (tmax, std::max (tx1, tx2));
    }

    if (_direction.y != 0.0f)
    {
      float const ty1 ((min.y - _origin.y) / _direction.y);
      float const ty2 ((max.y - _origin.y) / _direction.y);

      tmin = std::max (tmin, std::min (ty1, ty2));
      tmax = std::min (tmax, std::max (ty1, ty2));
    }

    if (_direction.z != 0.0f)
    {
      float const tz1 ((min.z - _origin.z) / _direction.z);
      float const tz2 ((max.z - _origin.z) / _direction.z);

      tmin = std::max (tmin, std::min (tz1, tz2));
      tmax = std::min (tmax, std::max (tz1, tz2));
    }

    if (tmax >= tmin)
    {
      return tmin;
    }

    return std::nullopt;
  }

  std::optional<float> ray::intersect_triangle
    (glm::vec3 const& v0, glm::vec3 const& v1, glm::vec3 const& v2) const
  {
      glm::vec3 e1 (v1 - v0);
      glm::vec3 e2 (v2 - v0);

      glm::vec3 P  = glm::cross(_direction,e2);

    float const det = glm::dot(e1, P);

    if (det == 0.0f)
    {
      return std::nullopt;
    }

    glm::vec3 const T (_origin - v0);
    float const dotu = glm::dot(T , P) / det;

    if (dotu < 0.0f || dotu > 1.0f)
    {
      return std::nullopt;
    }

    glm::vec3 const Q (glm::cross(T, e1));
    float const dotv = glm::dot(_direction , Q) / det;

    if (dotv < 0.0f || dotu + dotv > 1.0f)
    {
      return std::nullopt;
    }

    float const dott = glm::dot(e2 , Q) / det;

    if (dott > std::numeric_limits<float>::min())
    {
      return dott;
    }

    return std::nullopt;
  }
}
