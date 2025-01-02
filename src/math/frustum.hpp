// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <glm/mat4x4.hpp>
#include <array>

namespace math
{
  struct sphere;

  class frustum
  {
    enum SIDES
    {
      RIGHT,
      LEFT,
      BOTTOM,
      TOP,
      BACK,
      FRONT,
      SIDES_MAX,
    };

    class plane
    {
    public:
      plane() = default;
      plane (glm::vec4 const& vec)
        : _normal (glm::vec3(vec.x, vec.y, vec.z))
        , _distance (vec.w)
      {
        //normalize();
      }

      void normalize()
      {
        constexpr float recip (1.0f / glm::vec3::length());
        _normal *= recip;
        _distance *= recip;
      }

      const float& distance() const
      {
        return _distance;
      }

      const glm::vec3& normal() const
      {
        return _normal;
      }

    private:
      glm::vec3 _normal;
      float _distance;
    };
    std::array<plane, SIDES_MAX> _planes;

  public:
    frustum (glm::mat4x4 const& matrix);

    bool contains (const glm::vec3& point) const;
    bool intersects (const std::array<glm::vec3, 8>& intersect_points) const;
    bool intersects ( const glm::vec3& v1
                    , const glm::vec3& v2
                    ) const;
    bool intersectsSphere ( const glm::vec3& position
                          , const float& radius
                          ) const;

    bool intersectsSphere(sphere const& sphere) const;
  };
}
