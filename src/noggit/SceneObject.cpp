// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SceneObject.hpp"

#include <glm/gtx/quaternion.hpp>
#include <noggit/Misc.h>
#include <math/trig.hpp>

SceneObject::SceneObject(SceneObjectTypes type, noggit::NoggitRenderContext context, std::string filename)
: _type(type)
, _filename(filename)
, _context(context)
{
}

bool SceneObject::isInsideRect(std::array<glm::vec3, 2> const* rect) const
{
  return misc::rectOverlap(extents.data(), rect->data());
}

bool SceneObject::isDuplicateOf(SceneObject const& other)
{
  return _filename == other._filename
         && misc::vec3d_equals(pos, other.pos)
         && misc::deg_vec3d_equals(dir, other.dir)
         && misc::float_equals(scale, other.scale);
}

void SceneObject::updateTransformMatrix()
{
    auto rotationVector = glm::vec3(0);
    rotationVector.x = glm::radians(dir.x);
    rotationVector.y = glm::radians(dir.y - math::degrees(90.0)._);
    rotationVector.z = glm::radians(dir.z);

    auto matrix = glm::mat4x4(1);
    matrix = glm::translate(matrix, pos);
    glm::quat roationQuat = glm::quat(rotationVector);
	matrix = matrix * glm::toMat4(roationQuat);
    matrix = glm::scale(matrix, glm::vec3(scale, scale, scale));

   _transform_mat = matrix;
   _transform_mat_inverted = glm::inverse(matrix);
   _transform_mat_transposed = matrix;
}

void SceneObject::resetDirection()
{
  dir =  math::degrees::vec3(math::degrees(0)._, dir.y, math::degrees(0)._);
  recalcExtents();
}

void SceneObject::refTile(MapTile* tile)
{
  auto it = std::find(_tiles.begin(), _tiles.end(), tile);
  if (it == _tiles.end())
    _tiles.push_back(tile);
}

void SceneObject::derefTile(MapTile* tile)
{
  auto it = std::find(_tiles.begin(), _tiles.end(), tile);
  if (it != _tiles.end())
    _tiles.erase(it);
}
