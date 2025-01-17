// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>
#include <noggit/Misc.h>

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

  float aabb::volume()
  {
    float length = max.x - min.x;
    float width = max.y - min.y;
    float height = max.z - min.z;
    return length * width * height;
  }

  std::array<glm::vec3, 8> aabb::rotated_corners(glm::mat4x4 const& transform_mat, bool adjust_coords) const
  {
    std::array<glm::vec3, 8> corners_in_world;

    // fix coords for bounding boxes directly from game files that don't use noggit coords system
    if (adjust_coords)
    {
      auto transform = misc::transform_model_box_coords;
      const std::array<glm::vec3, 8> corners = all_corners();

      for (int i = 0; i < 8; ++i)
      {
        corners_in_world[i] = transform(corners[i]);
      }
    }
    else
    {
      corners_in_world = all_corners();
    }

    std::array<glm::vec3, 8> rotated_corners_in_world;
    // convert local to world
    for (int i = 0; i < 8; ++i)
    {
      rotated_corners_in_world[i] = transform_mat * glm::vec4(corners_in_world[i], 1.f);
    }

    return rotated_corners_in_world;
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

  float calculateOBBRadius(std::array<glm::vec3, 8> const& corners)
  {
    // Calculate the center point of the OBB
    glm::vec3 center(0.0f);
    for (const glm::vec3& corner : corners)
    {
      center += corner;
    }
    center /= 8.0f;  // Average of the corner points

    // Find the maximum distance from the center to any corner
    float maxDistance = 0.0f;
    for (const glm::vec3& corner : corners)
    {
      float distance = glm::distance(center, corner);
      maxDistance = std::max(maxDistance, distance);
    }

    return maxDistance;
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
