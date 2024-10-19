// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace math
{
  struct aabb
  {
    aabb(glm::vec3 const& min_, glm::vec3 const& max_);
    aabb(std::vector<glm::vec3> const& points);

    std::array<glm::vec3, 8> all_corners() const;
    inline glm::vec3 center() const { return (min + max) * 0.5f; };

    float volume();

    // Warning : this only works with local coords
    std::array<glm::vec3, 8> rotated_corners(glm::mat4x4 const& transform_mat, bool adjust_coords) const;

    glm::vec3 min;
    glm::vec3 max;
  };

  std::array<glm::vec3, 8> box_points(glm::vec3 const& box_min, glm::vec3 const& box_max);

  // 2d bounding box, aka square
  struct aabb_2d
  {
    aabb_2d(std::vector<glm::vec2> const& points);

    // std::array<glm::vec2, 4> all_corners() const;

    glm::vec2 min;
    glm::vec2 max;
  };

  float calculateOBBRadius(std::array<glm::vec3, 8> const& corners);
}
