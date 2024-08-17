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
  auto matrix = glm::mat4x4(1);
  matrix = glm::translate(matrix, pos);
  matrix = matrix * glm::eulerAngleYZX(glm::radians(dir.y - math::degrees(90.0)._), glm::radians(-dir.x), glm::radians(dir.z));
  matrix = glm::scale(matrix, glm::vec3(scale, scale, scale));

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
  auto it = std::find(_tiles.begin(), _tiles.end(), tile);
  if (it != _tiles.end())
    _tiles.erase(it);
}
