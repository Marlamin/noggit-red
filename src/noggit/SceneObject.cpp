// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SceneObject.hpp"
#include <noggit/Misc.h>
#include <math/trig.hpp>

SceneObject::SceneObject(SceneObjectTypes type, noggit::NoggitRenderContext context, std::string filename)
: _type(type)
, _filename(filename)
, _context(context)
{
}

bool SceneObject::isInsideRect(glm::vec3 rect[2]) const
{
  return misc::rectOverlap(extents, rect);
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
  math::matrix_4x4 mat( math::matrix_4x4(math::matrix_4x4::translation, pos)
                        * math::matrix_4x4
                            ( math::matrix_4x4::rotation_yzx
                                , { -dir.z
                                  , dir.y - math::degrees(90.0)._
                                  , dir.x
                              })

                          * math::matrix_4x4 (math::matrix_4x4::scale, scale)
  );

  _transform_mat = mat;
  _transform_mat_inverted = mat.inverted();
  _transform_mat_transposed = mat.transposed();
}

void SceneObject::resetDirection()
{
  dir =  math::degrees::vec3((0_deg)._, dir.y, (0.0_deg)._);
  recalcExtents();
}

void SceneObject::refTile(MapTile* tile)
{
  _tiles.emplace(tile);
}

void SceneObject::derefTile(MapTile* tile)
{
  _tiles.erase(tile);
}
