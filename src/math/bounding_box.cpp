// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <glm/common.hpp>
#include <math/bounding_box.hpp>

namespace
{
  glm::vec3 min_per_dimension(std::vector<glm::vec3> const& points)
  {
    auto min(glm::vec3( 
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(), 
        std::numeric_limits<float>::max() 
    ));

    for (auto const& point : points)
    {
      min = glm::min(min, point);
    }
    return min;
  }
  glm::vec3 max_per_dimension(std::vector<glm::vec3> const& points)
  {
    auto max(glm::vec3(
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
    ));
    for (auto const& point : points)
    {
      max = glm::max(max, point);
    }
    return max;
  }
}

namespace math
{
  aabb::aabb(glm::vec3 const& min_, glm::vec3 const& max_)
    : min(min_)
    , max(max_)
  {
  }

  aabb::aabb(std::vector<glm::vec3> const& points)
    : aabb(min_per_dimension(points), max_per_dimension(points))
  {
    assert(!points.empty());
  }

  //! \todo Optimize: iterate lazily.
  std::array<glm::vec3, 8> aabb::all_corners() const
  {
    return box_points(min, max);
  }


  std::array<glm::vec3, 8> box_points(glm::vec3 const& box_min, glm::vec3 const& box_max)
  {
    return std::array<glm::vec3, 8> {
        glm::vec3(box_max.x, box_max.y, box_max.z), // 0: Top-right-front
        glm::vec3(box_max.x, box_max.y, box_min.z), // 1: Top-right-back
        glm::vec3(box_max.x, box_min.y, box_max.z), // 2: Bottom-right-front
        glm::vec3(box_max.x, box_min.y, box_min.z), // 3: Bottom-right-back
        glm::vec3(box_min.x, box_max.y, box_max.z), // 4: Top-left-front
        glm::vec3(box_min.x, box_max.y, box_min.z), // 5: Top-left-back
        glm::vec3(box_min.x, box_min.y, box_max.z), // 6: Bottom-left-front
        glm::vec3(box_min.x, box_min.y, box_min.z)  // 7: Bottom-left-back
    };
  }

  aabb_2d::aabb_2d(std::vector<glm::vec2> const& points)
  {
    assert(!points.empty());

    min = glm::vec2(
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max()
    );

    max = glm::vec2(
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest()
    );

    for (auto const& point : points)
    {
      min = glm::min(min, point);
      max = glm::max(max, point);
    }
  }
}
