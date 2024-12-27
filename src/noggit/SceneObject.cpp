// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SceneObject.hpp"

#include <ClientData.hpp>
#include <noggit/AsyncObject.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <noggit/Misc.h>
#include <math/trig.hpp>
#include <limits>

SceneObject::SceneObject(SceneObjectTypes type, Noggit::NoggitRenderContext context)
: _type(type)
, _context(context)
, pos(0.f, 0.f, 0.f)
, dir(0.f, 0.f, 0.f)
, uid(0)
, frame(0)
{
  // min and max initialized to their opposites
  extents[0] = glm::vec3(std::numeric_limits<float>::max()); 
  extents[1] = glm::vec3(std::numeric_limits<float>::lowest()); 
}

bool SceneObject::isInsideRect(std::array<glm::vec3, 2> const* rect) const
{
  return misc::rectOverlap(extents.data(), rect->data());
}

bool SceneObject::isDuplicateOf(SceneObject const& other)
{
  auto a_obj_this = instance_model();
  auto a_obj_other = other.instance_model();

  if ((a_obj_this && a_obj_other) && a_obj_this->file_key() != a_obj_other->file_key())
  {
    return false;
  }

  // if one SceneObject has an AsyncObject bound and ther other is not,
  // there is no point in comparing further
  if (static_cast<bool>(a_obj_this) != static_cast<bool>(a_obj_other))
  {
    return false;
  }

  return misc::vec3d_equals(pos, other.pos)
         && misc::deg_vec3d_equals(dir, other.dir)
         && misc::float_equals(scale, other.scale);
}

void SceneObject::updateTransformMatrix()
{
  glm::mat4x4 matrix = glm::mat4x4(1.0f);

  if (pos != glm::vec3(0.0f))
  {
    matrix = glm::translate(matrix, pos);
  }

  // Normalize small direction values and handle -0.0f
  glm::vec3 clamped_dir = glm::vec3(
    (glm::abs(dir.x) < 1e-6f || dir.x == -0.0f) ? 0.0f : dir.x,
    (glm::abs(dir.y) < 1e-6f || dir.y == -0.0f) ? 0.0f : dir.y,
    (glm::abs(dir.z) < 1e-6f || dir.z == -0.0f) ? 0.0f : dir.z
  );

  // always need to recalc rotation because of the angles, maybe we can initialize matrix to -90 etc...
  // if (clamped_dir != glm::vec3(0.0f))
  {
    matrix *= glm::eulerAngleYZX(
      glm::radians(clamped_dir.y - math::degrees(90.0f)._),
      glm::radians(-clamped_dir.x),
      glm::radians(clamped_dir.z)
    );
  }

  if (scale != 1.0f)
  {
    matrix = glm::scale(matrix, glm::vec3(scale, scale, scale));
  }

  _transform_mat = matrix;
  _transform_mat_inverted = glm::inverse(matrix);
}

void SceneObject::resetDirection()
{
  dir =  math::degrees::vec3(math::degrees(0)._, dir.y, math::degrees(0)._);
  recalcExtents();
}

void SceneObject::normalizeDirection()
{
    //! [-180, 180)
    while (dir.x >= 180.0f)
        dir.x -= 360.0f;
    while (dir.x < -180.0f)
        dir.x += 360.0f;

    while (dir.y >= 180.0f)
        dir.y -= 360.0f;
    while (dir.y < -180.0f)
        dir.y += 360.0f;

    while (dir.z >= 180.0f)
        dir.z -= 360.0f;
    while (dir.z < -180.0f)
        dir.z += 360.0f;
}

void SceneObject::refTile(MapTile* tile)
{
  assert(tile);
  auto it = std::find(_tiles.begin(), _tiles.end(), tile);
  if (it == _tiles.end())
    _tiles.push_back(tile);
}

void SceneObject::derefTile(MapTile* tile)
{
  assert(tile);
  if (_tiles.empty())
  {
    return;
  }

  auto it = std::find(_tiles.begin(), _tiles.end(), tile);
  if (it != _tiles.end())
    _tiles.erase(it);
}
