// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_3DOBJECT_HPP
#define NOGGIT_3DOBJECT_HPP

#include <math/vector_3d.hpp>
#include <math/matrix_4x4.hpp>
#include <math/ray.hpp>
#include <noggit/Selection.h>
#include <noggit/ContextObject.hpp>
#include <cstdint>
#include <unordered_set>

class AsyncObject;

enum SceneObjectTypes
{
  eMODEL,
  eWMO
};

class MapTile;

class SceneObject
{
public:
  SceneObject(SceneObjectTypes type, noggit::NoggitRenderContext context, std::string filename = "");

  [[nodiscard]]
  bool isInsideRect(math::vector_3d rect[2]) const;

  [[nodiscard]]
  bool isDuplicateOf(SceneObject const& other);

  virtual void updateTransformMatrix();

  virtual void recalcExtents() = 0;
  virtual void ensureExtents() = 0;
  virtual bool finishedLoading() = 0;

  void resetDirection();

  [[nodiscard]]
  math::matrix_4x4 transformMatrix() const { return _transform_mat; };

  [[nodiscard]]
  math::matrix_4x4 transformMatrixInverted() const { return _transform_mat_inverted; };

  [[nodiscard]]
  math::matrix_4x4 transformMatrixTransposed() const { return _transform_mat_transposed; };

  SceneObjectTypes which() { return _type; };

  std::string const& getFilename() { return _filename; };

  void refTile(MapTile* tile);
  void derefTile(MapTile* tile);
  std::unordered_set<MapTile*> const& getTiles() { return _tiles; };

  virtual AsyncObject* instance_model() = 0;

public:
  math::vector_3d pos;
  math::vector_3d  extents[2];
  math::degrees::vec3 dir;
  float scale = 1.f;
  unsigned int uid;
  int frame;

protected:
  SceneObjectTypes _type;

  math::matrix_4x4 _transform_mat = math::matrix_4x4::uninitialized;
  math::matrix_4x4 _transform_mat_inverted = math::matrix_4x4::uninitialized;
  math::matrix_4x4 _transform_mat_transposed = math::matrix_4x4::uninitialized;

  noggit::NoggitRenderContext _context;

  std::string _filename;

  std::unordered_set<MapTile*> _tiles;
};

#endif //NOGGIT_3DOBJECT_HPP
