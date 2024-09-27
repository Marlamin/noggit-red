// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <ClientData.hpp>

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace misc
{

  bool pointInside(glm::vec3 point, std::array<glm::vec3, 2> const& extents)
  {
    return point.x >= extents[0].x && point.z >= extents[0].z &&
           point.x <= extents[1].x && point.z <= extents[1].z;
  }

  bool pointInside(glm::vec2 point, std::array<glm::vec2, 2> const& extents)
  {
    return point.x >= extents[0].x && point.y >= extents[0].y &&
           point.x <= extents[1].x && point.y <= extents[1].y;
  }

  // project 3D point to 2d screen space.
  // Note : When working with noggit 3D coords, need to swap Y and Z !
  glm::vec4 projectPointToScreen(const glm::vec3& point, const glm::mat4& VPmatrix, float viewport_width, float viewport_height, bool& valid)
  {
    glm::vec4 clipSpacePos = VPmatrix * glm::vec4(point, 1.0f);

    if (clipSpacePos.w <= 0.0f)
    {
      valid = false;  // not valid, point is behind camera
      return clipSpacePos;
    }

    // Perspective division to move to normalized device coordinates (NDC)
    float ndcX = clipSpacePos.x / clipSpacePos.w;
    float ndcY = clipSpacePos.y / clipSpacePos.w;
    float ndcZ = clipSpacePos.z / clipSpacePos.w;

    // If the point is out of the normalized device coordinates range, it's off-screen
    if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f)
    {
        valid = false;
        return glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
    }

    // Convert NDC to screen space coordinates
    clipSpacePos.x = (ndcX + 1.0f) * 0.5f * viewport_width;
    clipSpacePos.y = (1.0f - (ndcY + 1.0f) * 0.5f) * viewport_height;

    // from MapView::normalized_device_coords
    // x 2.0f * x / viewport_width - 1.0f
    // y 1.0f - 2.0f * y / viewport_height
    // z 0.0f
    // w 1.0f

    valid = true;
    return clipSpacePos;
  }

  std::array<glm::vec2, 2> getAABBScreenBounds(const std::array<glm::vec3, 2>& extents
                                              , const glm::mat4& VPmatrix
                                              , float viewport_width
                                              , float viewport_height
                                              , bool& valid
                                              , float scale)
  {
    math::aabb obj_aabb(extents[0], extents[1]);
    auto corners = obj_aabb.all_corners();

    glm::vec2 minScreen = glm::vec2(std::numeric_limits<float>::max());
    glm::vec2 maxScreen = glm::vec2(std::numeric_limits<float>::lowest());

    for (const auto& corner : corners)
    {
      bool point_valid;
      const glm::vec4 screenPos = projectPointToScreen(corner, VPmatrix, viewport_width, viewport_height, point_valid);

      if (!point_valid)
      {
        valid = false; // one point was outside of screen space
        return { glm::vec2(0.0f), glm::vec2(0.0f) };
      }

      // Update min and max screen bounds if the point is valid (within screen space)
      minScreen = glm::min(minScreen, glm::vec2(screenPos.x, screenPos.y));
      maxScreen = glm::max(maxScreen, glm::vec2(screenPos.x, screenPos.y));
    }
    valid = true;

    if (scale != 1.0f)
    {
      glm::vec2 center = (minScreen + maxScreen) * 0.5f;
      glm::vec2 halfSize = (maxScreen - minScreen) * 0.5f * scale;

      minScreen = center - halfSize;
      maxScreen = center + halfSize;
    }

    return { minScreen, maxScreen };
  }

  void minmax(glm::vec3* a, glm::vec3* b)
  {
    if (a->x > b->x)
    {
      std::swap(a->x, b->x);
    }
    if (a->y > b->y)
    {
      std::swap(a->y, b->y);
    }
    if (a->z > b->z)
    {
      std::swap(a->z, b->z);
    }
  }

  void find_and_replace(std::string& source, const std::string& find, const std::string& replace)
  {
    size_t found = source.rfind(find);
    while (found != std::string::npos) //fixed unknown letters replace. Now it works correctly and replace all found symbold instead of only one at previous versions
    {
      source.replace(found, find.length(), replace);
      found = source.rfind(find);
    }
  }

  float frand()
  {
    return rand() / static_cast<float>(RAND_MAX);
  }

  float randfloat(float lower, float upper)
  {
    return lower + (upper - lower) * frand();
  }

  int randint(int lower, int upper)
  {
    return lower + static_cast<int>((upper + 1 - lower) * frand());
  }

  float dist(float x1, float z1, float x2, float z2)
  {
    float xdiff = x2 - x1, zdiff = z2 - z1;
    return std::sqrt(xdiff*xdiff + zdiff*zdiff);
  }

  float dist(glm::vec3 const& p1, glm::vec3 const& p2)
  {
    return dist(p1.x, p1.z, p2.x, p2.z);
  }

  // return the shortest distance between the point (x, z)
  // and square at (squareX, squareZ) with a size of unitSize
  float getShortestDist(float x, float z, float squareX, float squareZ, float unitSize)
  {
    float px, pz;

    if (x >= squareX && x < squareX + unitSize)
    {
      px = x;
    }
    else
    {
      px = (squareX < x) ? squareX + unitSize : squareX;
    }

    if (z >= squareZ && z < squareZ + unitSize)
    {
      pz = z;
    }
    else
    {
      pz = (squareZ < z) ? squareZ + unitSize : squareZ;
    }

    return (px == x && pz == z) ? 0.0f : dist(x, z, px, pz);
  }

  float getShortestDist(glm::vec3 const& pos, glm::vec3 const& square_pos, float unitSize)
  {
    return getShortestDist(pos.x, pos.z, square_pos.x, square_pos.z, unitSize);
  }

  bool square_is_in_circle(float x, float z, float radius, float square_x, float square_z, float square_size)
  {
    float px, pz;

    if (std::abs(square_x - x) < std::abs(square_x + square_size - x))
    {
      px = square_x + square_size;
    }
    else
    {
      px = square_x;
    }

    if (std::abs(square_z - z) < std::abs(square_z + square_size - z))
    {
      pz = square_z + square_size;
    }
    else
    {
      pz = square_z;
    }

    // check if the furthest is in the circle
    float d = dist(x, z, px, pz);
    return d <= radius;
  }

  bool rectOverlap(glm::vec3 const* r1, glm::vec3 const* r2)
  {
    return r1[0].x <= r2[1].x
      && r2[0].x <= r1[1].x
      && r1[0].z <= r2[1].z
      && r2[0].z <= r1[1].z;
  }

  float angledHeight(glm::vec3 const& origin, glm::vec3 const& pos, math::radians const& angle, math::radians const& orientation)
  {
    return ( origin.y
           + (  (pos.x - origin.x) * glm::cos(orientation._)
              + (pos.z - origin.z) * glm::sin(orientation._)
             ) * glm::tan(angle._));
  }

  void extract_v3d_min_max(glm::vec3 const& point, glm::vec3& min, glm::vec3& max)
  {
    min.x = std::min(min.x, point.x);
    max.x = std::max(max.x, point.x);
    min.y = std::min(min.y, point.y);
    max.y = std::max(max.y, point.y);
    min.z = std::min(min.z, point.z);
    max.z = std::max(max.z, point.z);
  }

  std::vector<glm::vec3> intersection_points(glm::vec3 const& vmin, glm::vec3 const& vmax)
  {
    std::vector<glm::vec3> points;

    points.emplace_back (vmin.x, vmin.y, vmin.z);
    points.emplace_back (vmin.x, vmin.y, vmax.z);
    points.emplace_back (vmin.x, vmax.y, vmin.z);
    points.emplace_back (vmin.x, vmax.y, vmax.z);
    points.emplace_back (vmax.x, vmin.y, vmin.z);
    points.emplace_back (vmax.x, vmin.y, vmax.z);
    points.emplace_back (vmax.x, vmax.y, vmin.z);
    points.emplace_back (vmax.x, vmax.y, vmax.z);

    return points;
  }

  glm::vec3 transform_model_box_coords(glm::vec3 const& pos)
  {
    return {pos.x, pos.z, -pos.y};
  }

  std::string normalize_adt_filename(std::string filename)
  {
    std::transform (filename.begin(), filename.end(), filename.begin(), ::toupper);
    std::transform ( filename.begin(), filename.end(), filename.begin()
                   , [](char c)
                     {
                       return c == '/' ? '\\' : c;
                     }
                   );
    return filename;
  }

  bool vec3d_equals(glm::vec3 const& v1, glm::vec3 const& v2)
  {
    return float_equals(v1.x, v2.x) && float_equals(v1.y, v2.y) && float_equals(v1.z, v2.z);
  }

  bool deg_vec3d_equals(math::degrees::vec3 const& v1, math::degrees::vec3 const& v2)
  {
    return float_equals(v1.x, v2.x) && float_equals(v1.y, v2.y) && float_equals(v1.z, v2.z);
  }
}

void SetChunkHeader(util::sExtendableArray& pArray, int pPosition, int pMagix, int pSize)
{
  auto const Header = pArray.GetPointer<sChunkHeader>(pPosition);
  Header->mMagic = pMagix;
  Header->mSize = pSize;
}

