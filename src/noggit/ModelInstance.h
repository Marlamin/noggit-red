// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <ClientFile.hpp>
#include <noggit/MapHeaders.h> // ENTRY_MDDF
#include <noggit/ModelManager.h>
#include <noggit/Selection.h>
#include <noggit/SceneObject.hpp>
#include <noggit/rendering/Primitives.hpp>
#include <noggit/TileIndex.hpp>
#include <noggit/tool_enums.hpp>
#include <opengl/shader.fwd.hpp>
#include <optional>
#include <cstdint>

namespace math { class frustum; }
class Model;
class WMOInstance;

class ModelInstance : public SceneObject
{
public:
  constexpr static float min_scale() { return 1.f / 1024.f; };
  constexpr static float max_scale() { return static_cast<float>((1 << 16) - 1) / 1024.f; };

  scoped_model_reference model;

  glm::vec3 light_color = { 1.f, 1.f, 1.f };

  // used when flag 0x8 is set in wdt
  // longest side of an AABB transformed model's bounding box from the M2 header
  float size_cat;

  explicit ModelInstance(BlizzardArchive::Listfile::FileKey const& file_key
                         , Noggit::NoggitRenderContext context);

  explicit ModelInstance(BlizzardArchive::Listfile::FileKey const& file_key
                         , ENTRY_MDDF const*d, Noggit::NoggitRenderContext context);

  ModelInstance(ModelInstance const& other) = default;
  ModelInstance& operator= (ModelInstance const& other) = default;

  ModelInstance (ModelInstance&& other)
    : SceneObject(other._type, other._context)
    , model (std::move (other.model))
    , light_color (other.light_color)
    , size_cat (other.size_cat)
    , _need_recalc_extents(other._need_recalc_extents)
  {
    pos = other.pos;
    dir = other.dir;
    scale = other.scale;
    extents[0] = other.extents[0];
    extents[1] = other.extents[1];
    _transform_mat_inverted =  other._transform_mat_inverted;
    _context = other._context;
    uid = other.uid;
  }

  ModelInstance& operator= (ModelInstance&& other) noexcept
  {
    std::swap (model, other.model);
    std::swap (pos, other.pos);
    std::swap (dir, other.dir);
    std::swap (light_color, other.light_color);
    std::swap (uid, other.uid);
    std::swap (scale, other.scale);
    std::swap (size_cat, other.size_cat);
    std::swap (_need_recalc_extents, other._need_recalc_extents);
    std::swap (extents, other.extents);
    std::swap(_transform_mat_inverted, other._transform_mat_inverted);
    std::swap(_context, other._context);
    return *this;
  }

  void draw_box (glm::mat4x4 const& model_view
                , glm::mat4x4 const& projection
                , bool is_current_selection
                );


  std::vector<std::tuple<int, int, int>> intersect(glm::mat4x4 const& model_view
      , math::ray const&
      , selection_result*
      , int animtime
  );

  bool isInFrustum(math::frustum const& frustum);
  bool isInRenderDist(const float& cull_distance, const glm::vec3& camera, display_mode display);

  [[nodiscard]]
  virtual glm::vec3 const& get_pos() const { return pos; }

  void recalcExtents() override;
  void ensureExtents() override;
  bool finishedLoading() override { return model->finishedLoading(); };
  std::array<glm::vec3, 2> const& getExtents() override;
  // glm::vec3* getExtents();

  [[nodiscard]]
  virtual bool isWMODoodad() const { return false; };

  [[nodiscard]]
  AsyncObject* instance_model() const override { return model.get(); };

  void updateDetails(Noggit::Ui::detail_infos* detail_widget) override;

  [[nodiscard]]
  std::uint32_t gpuTransformUid() const { return _gpu_transform_uid; }

protected:
  bool _need_recalc_extents = true;
  bool _need_gpu_transform_update = true;
  std::uint32_t _gpu_transform_uid = 0;

};

class wmo_doodad_instance : public ModelInstance
{
public:
  glm::quat doodad_orientation;
  glm::vec3 world_pos;
  // local wmo group position is stored in modelInstance::pos
  // same for scale

  explicit wmo_doodad_instance(BlizzardArchive::Listfile::FileKey const& file_key
      , BlizzardArchive::ClientFile* f
      , Noggit::NoggitRenderContext context );

  // titi : issue with those constructors: ModelInstance data is lost (scale, pos...)
  /**/
  wmo_doodad_instance(wmo_doodad_instance const& other)
  // : ModelInstance(other.model->file_key(), other._context)
  : ModelInstance(other)  // titi : Use the copy constructor of ModelInstance instead
  , doodad_orientation(other.doodad_orientation)
  , world_pos(other.world_pos)
  , _need_matrix_update(other._need_matrix_update)
  {
      // titi: added those.
      // pos = other.pos;
      // scale = other.scale;
      // frame = other.frame;
  };

  wmo_doodad_instance& operator= (wmo_doodad_instance const& other) = delete;

  wmo_doodad_instance(wmo_doodad_instance&& other) noexcept
    : ModelInstance(reinterpret_cast<ModelInstance&&>(other))
    , doodad_orientation(other.doodad_orientation)
    , world_pos(other.world_pos)
    , _need_matrix_update(other._need_matrix_update)
  {
  }

  wmo_doodad_instance& operator= (wmo_doodad_instance&& other)
  {
    ModelInstance::operator= (reinterpret_cast<ModelInstance&&>(other));
    std::swap (doodad_orientation, other.doodad_orientation);
    std::swap (world_pos, other.world_pos);
    std::swap (_need_matrix_update, other._need_matrix_update);
    return *this;
  }
  

  [[nodiscard]]
  bool need_matrix_update() const { return _need_matrix_update; }

  void update_transform_matrix_wmo(WMOInstance* wmo);

  [[nodiscard]]
  glm::vec3 const& get_pos() const override { return world_pos; };

  [[nodiscard]]
  bool isWMODoodad() const override { return true; };

protected:
  // to avoid redefining recalcExtents
  void updateTransformMatrix() override { }

private:
  bool _need_matrix_update = true;
};
