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
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min()
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

  aabb::aabb(std::vector<glm::vec3> points)
    : aabb(min_per_dimension(points), max_per_dimension(points))
  {
  }

  //! \todo Optimize: iterate lazily.
  std::vector<glm::vec3> aabb::all_corners() const
  {
    return box_points(min, max);
  }


  std::vector<glm::vec3> box_points(glm::vec3 const& box_min, glm::vec3 const& box_max)
  {
    std::vector<glm::vec3> points;

    points.emplace_back(box_max.x, box_max.y, box_max.z);
    points.emplace_back(box_max.x, box_max.y, box_min.z);
    points.emplace_back(box_max.x, box_min.y, box_max.z);
    points.emplace_back(box_max.x, box_min.y, box_min.z);
    points.emplace_back(box_min.x, box_max.y, box_max.z);
    points.emplace_back(box_min.x, box_max.y, box_min.z);
    points.emplace_back(box_min.x, box_min.y, box_max.z);
    points.emplace_back(box_min.x, box_min.y, box_min.z);

    return points;
  }
}
