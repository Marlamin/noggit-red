// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ModelManager.h>
#include <noggit/Selection.h>
#include <noggit/SceneObject.hpp>
#include <noggit/tool_enums.hpp>

#include <glm/vec3.hpp>
#include <glm/ext.hpp>

#include <cstdint>

namespace math
{
  class frustum;
  struct ray;
}

namespace BlizzardArchive
{
  class ClientFile;

  namespace Listfile
  {
    class FileKey;
  }
}

struct ENTRY_MDDF;
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
  float size_cat = 0.0f;

  explicit ModelInstance(BlizzardArchive::Listfile::FileKey const& file_key
                         , Noggit::NoggitRenderContext context);

  explicit ModelInstance(BlizzardArchive::Listfile::FileKey const& file_key
                         , ENTRY_MDDF const*d, Noggit::NoggitRenderContext context);

  ModelInstance(ModelInstance const& other) = default;
  ModelInstance& operator= (ModelInstance const& other) = default;

  ModelInstance (ModelInstance&& other) noexcept;

  ModelInstance& operator= (ModelInstance&& other) noexcept;

  void draw_box (glm::mat4x4 const& model_view
                , glm::mat4x4 const& projection
                , bool is_current_selection
                , bool draw_collision_box
                , bool draw_aabb
                , bool draw_anim_bb
                );


 void intersect(glm::mat4x4 const& model_view
      , math::ray const&
      , selection_result*
      , int animtime
      , bool animate
  );

  bool isInFrustum(math::frustum const& frustum);
  bool isInRenderDist(const float cull_distance, const glm::vec3& camera, display_mode display);

  bool extentsDirty() const;;

  [[nodiscard]]
  virtual glm::vec3 const& get_pos() const;

  void recalcExtents() override;
  void ensureExtents() override;
  bool finishedLoading() override;;
  std::array<glm::vec3, 2> const& getExtents() override; // axis aligned
  std::array<glm::vec3, 2> const& getLocalExtents() const;

  std::array<glm::vec3, 8> getBoundingBox() override; // not axis aligned

  [[nodiscard]]
  virtual bool isWMODoodad() const;;

  [[nodiscard]]
  AsyncObject* instance_model() const override;;

  void updateDetails(Noggit::Ui::detail_infos* detail_widget) override;

  [[nodiscard]]
  std::uint32_t gpuTransformUid() const;

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
  wmo_doodad_instance(wmo_doodad_instance const& other);;

  wmo_doodad_instance& operator= (wmo_doodad_instance const& other) = delete;

  wmo_doodad_instance(wmo_doodad_instance&& other) noexcept;

  wmo_doodad_instance& operator= (wmo_doodad_instance&& other) noexcept;

  [[nodiscard]]
  bool need_matrix_update() const;

  void update_transform_matrix_wmo(WMOInstance* wmo);

  bool isInRenderDist(const float cull_distance, const glm::vec3& camera, display_mode display);

  [[nodiscard]]
  glm::vec3 const& get_pos() const override;;

  [[nodiscard]]
  bool isWMODoodad() const override;;

protected:
  // to avoid redefining recalcExtents
  void updateTransformMatrix() override;

private:
  bool _need_matrix_update = true;
};
