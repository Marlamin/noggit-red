// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>

#include <vector>

namespace math
{
  struct aabb
  {
    aabb(glm::vec3 const& min_, glm::vec3 const& max_);
    aabb(std::vector<glm::vec3> points);

    std::vector<glm::vec3> all_corners() const;

    glm::vec3 min;
    glm::vec3 max;
  };

  std::vector<glm::vec3> box_points(glm::vec3 const& box_min, glm::vec3 const& box_max);
}
