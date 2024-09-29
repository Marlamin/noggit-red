// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/ray.hpp>

#include <limits>
#include <algorithm>

namespace math
{
  std::optional<float> ray::intersect_bounds
    (glm::vec3 const& min, glm::vec3 const& max) const noexcept
  {
    float tmin (std::numeric_limits<float>::lowest());
    float tmax (std::numeric_limits<float>::max());

    auto calculate_tmin_tmax = [](float origin, float direction, float min, float max, float& tmin, float& tmax) {
      if (direction != 0.0f) {
        float t1 = (min - origin) / direction;
        float t2 = (max - origin) / direction;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
      }
      };

    calculate_tmin_tmax(_origin.x, _direction.x, min.x, max.x, tmin, tmax);
    calculate_tmin_tmax(_origin.y, _direction.y, min.y, max.y, tmin, tmax);
    calculate_tmin_tmax(_origin.z, _direction.z, min.z, max.z, tmin, tmax);

    if (tmax >= tmin)
    {
      return tmin;
    }

    return std::nullopt;
  }

  std::optional<float> ray::intersect_triangle
    (glm::vec3 const& v0, glm::vec3 const& v1, glm::vec3 const& v2) const noexcept
  {
      glm::vec3 e1 (v1 - v0);
      glm::vec3 e2 (v2 - v0);

      glm::vec3 P  = glm::cross(_direction,e2);

    float const det = glm::dot(e1, P);

    constexpr float epsilon = std::numeric_limits<float>::epsilon();
    if (std::fabs(det) < epsilon) // if (det == 0.0f)
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

    if (dott > epsilon) // if (dott > std::numeric_limits<float>::min())
    {
      return dott;
    }

    return std::nullopt;
  }
}
