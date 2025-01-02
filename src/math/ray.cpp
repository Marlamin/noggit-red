// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/ray.hpp>
#include <math/sphere.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/matrix.hpp>

#include <limits>
#include <algorithm>

namespace math
{
  float magnitutde_sq(glm::vec2 const& v)
  {
    return glm::dot(v, v);
  }

  void calculate_tmin_tmax(float origin, float direction, float min, float max, float& tmin, float& tmax)
  {
    if (direction != 0.0f) {
      float t1 = (min - origin) / direction;
      float t2 = (max - origin) / direction;
      tmin = std::max(tmin, std::min(t1, t2));
      tmax = std::min(tmax, std::max(t1, t2));
    }
  }

  std::optional<float> ray::intersect_bounds
    (glm::vec3 const& min, glm::vec3 const& max) const noexcept
  {
    float tmin(std::numeric_limits<float>::lowest());
    float tmax(std::numeric_limits<float>::max());

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
    glm::vec3 e1(v1 - v0);
    glm::vec3 e2(v2 - v0);

    glm::vec3 P = glm::cross(_direction, e2);

    float const det = glm::dot(e1, P);

    constexpr float epsilon = std::numeric_limits<float>::epsilon();
    if (std::fabs(det) < epsilon) // if (det == 0.0f)
    {
      return std::nullopt;
    }

    glm::vec3 const T(_origin - v0);
    float const dotu = glm::dot(T, P) / det;

    if (dotu < 0.0f || dotu > 1.0f)
    {
      return std::nullopt;
    }

    glm::vec3 const Q(glm::cross(T, e1));
    float const dotv = glm::dot(_direction, Q) / det;

    if (dotv < 0.0f || dotu + dotv > 1.0f)
    {
      return std::nullopt;
    }

    float const dott = glm::dot(e2, Q) / det;

    if (dott > epsilon) // if (dott > std::numeric_limits<float>::min())
    {
      return dott;
    }

    return std::nullopt;
  }

  std::optional<float> ray::intersect_box
    (glm::vec3 const& position, glm::vec3 const& box_min, glm::vec3 const& box_max, glm::vec3 const& rotation) const noexcept
  {
    auto inverse_rotation = glm::inverse(glm::yawPitchRoll(rotation.y, rotation.z, rotation.x));

    glm::vec3 origin = inverse_rotation * glm::vec4{ _origin - position, 1.f };
    glm::vec3 direction = inverse_rotation * glm::vec4{ _direction, 0.0f };

    float tmin(std::numeric_limits<float>::lowest());
    float tmax(std::numeric_limits<float>::max());

    calculate_tmin_tmax(origin.x, direction.x, box_min.x, box_max.x, tmin, tmax);
    calculate_tmin_tmax(origin.y, direction.y, box_min.y, box_max.y, tmin, tmax);
    calculate_tmin_tmax(origin.z, direction.z, box_min.z, box_max.z, tmin, tmax);

    if (tmax >= tmin)
    {
      return tmin;
    }

    return std::nullopt;
  }

  hit_result ray::intersects_sphere(sphere const& sphere) const
  {
    // Taken from https://gamedev.stackexchange.com/a/96469
    
    // Calculate ray start's offset from the sphere center
    glm::vec3 p = _origin - sphere.position;

    float rSquared = sphere.radius * sphere.radius;
    float p_d = dot(p, _direction);

    // The sphere is behind or surrounding the start point.
    if (p_d > 0 || dot(p, p) < rSquared)
      return { sphere.position, sphere.radius, false };

    // Flatten p into the plane passing through sphere.position perpendicular to the ray.
    // This gives the closest approach of the ray to the center.
    glm::vec3 a = p - p_d * _direction;

    float aSquared = dot(a, a);

    // Closest approach is outside the sphere.
    if (aSquared > rSquared)
      return { sphere.position, sphere.radius, false };

    // Calculate distance from plane where ray enters/exits the sphere.    
    float h = sqrt(rSquared - aSquared);

    // Calculate intersection point relative to sphere center.
    glm::vec3 i = a - h * _direction;

    glm::vec3 intersection = sphere.position + i;
    glm::vec3 normal = i / sphere.radius;
    // We've taken a shortcut here to avoid a second square root.
    // Note numerical errors can make the normal have length slightly different from 1.
    // If you need higher precision, you may need to perform a conventional normalization.

    return { intersection, h, true };
  }
}
