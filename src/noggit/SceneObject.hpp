// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_3DOBJECT_HPP
#define NOGGIT_3DOBJECT_HPP

#include <glm/mat4x4.hpp>
#include <math/ray.hpp>
#include <noggit/Selection.h>
#include <noggit/ContextObject.hpp>
#include <cstdint>
#include <unordered_set>
#include <array>

namespace BlizzardArchive::Listfile
{
  class FileKey;
}

class AsyncObject;

enum SceneObjectTypes
{
  eMODEL,
  eWMO
};

class MapTile;

class SceneObject : public Selectable
{
public:
  SceneObject(SceneObjectTypes type, Noggit::NoggitRenderContext context);

  [[nodiscard]]
  bool isInsideRect(std::array<glm::vec3, 2> const* rect) const;

  [[nodiscard]]
  bool isDuplicateOf(SceneObject const& other);

  virtual void updateTransformMatrix();

  virtual void recalcExtents() = 0;
  virtual void ensureExtents() = 0;

  [[nodiscard]]
  virtual bool finishedLoading() = 0;

  void resetDirection();

  void normalizeDirection();

  [[nodiscard]]
  glm::mat4x4 transformMatrix() const { return _transform_mat; };

  [[nodiscard]]
  glm::mat4x4 transformMatrixInverted() const { return _transform_mat_inverted; };

  [[nodiscard]]
  SceneObjectTypes which() const { return _type; };

  void refTile(MapTile* tile);
  void derefTile(MapTile* tile);

  [[nodiscard]]
  std::vector<MapTile*> const& getTiles() const { return _tiles; };

  [[nodiscard]]
  virtual AsyncObject* instance_model() const = 0;

  [[nodiscard]]
  std::array<glm::vec3, 2> const& getExtents() { ensureExtents(); return extents; }

public:
  glm::vec3 pos;
  std::array<glm::vec3, 2> extents;
  glm::vec3 dir;
  float scale = 1.f;
  unsigned int uid;
  int frame;

protected:
  SceneObjectTypes _type;

  glm::mat4x4 _transform_mat = glm::mat4x4();
  glm::mat4x4 _transform_mat_inverted = glm::mat4x4();

  Noggit::NoggitRenderContext _context;

  std::vector<MapTile*> _tiles;
};

#endif //NOGGIT_3DOBJECT_HPP
