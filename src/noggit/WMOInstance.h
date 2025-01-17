// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/SceneObject.hpp>
#include <noggit/WMO.h>
#include <noggit/ContextObject.hpp>

#include <cstdint>

namespace math
{
  struct ray;
}

struct ENTRY_MODF;

class WMOInstance : public SceneObject
{
public:
  scoped_wmo_reference wmo;

  uint16_t mFlags;
  uint16_t mNameset; 

  uint16_t doodadset() const;
  void change_doodadset(uint16_t doodad_set);

  [[nodiscard]]
  std::map<int, std::pair<glm::vec3, glm::vec3>> const& getGroupExtents();

private:
  void update_doodads();

  uint16_t _doodadset;
  std::map<int, std::pair<glm::vec3, glm::vec3>> group_extents;
  std::map<uint32_t, std::vector<wmo_doodad_instance>> _doodads_per_group;
  bool _need_doodadset_update = true;
  bool _update_group_extents = false;
  bool _need_recalc_extents = true;

public:
  WMOInstance(BlizzardArchive::Listfile::FileKey const& file_key, ENTRY_MODF const* d, Noggit::NoggitRenderContext context);

  explicit WMOInstance(BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context);

  WMOInstance(WMOInstance const& other) = default;
  WMOInstance& operator=(WMOInstance const& other) = default;

  WMOInstance (WMOInstance&& other) noexcept;

  WMOInstance& operator= (WMOInstance&& other) noexcept;

  void draw ( OpenGL::Scoped::use_program& wmo_shader
            , glm::mat4x4 const& model_view
            , glm::mat4x4 const& projection
            , math::frustum const& frustum
            , const float& cull_distance
            , const glm::vec3& camera
            , bool force_box
            , bool draw_doodads
            , bool draw_fog
            , bool is_selected
            , int animtime
            , bool world_has_skies
            , display_mode display
            , bool no_cull
            , bool draw_exterior
            , bool render_selection_aabb
            , bool render_group_bounds
            );

  void intersect (math::ray const&, selection_result*, bool do_exterior = true);

  std::array<glm::vec3, 2> const& getExtents() override; // axis aligned
  std::array<glm::vec3, 2> const& getLocalExtents() const;
  std::array<glm::vec3, 8> getBoundingBox() override; // not axis aligned
  bool extentsDirty() const;;
  void recalcExtents() override;
  void change_nameset(uint16_t name_set);
  void ensureExtents() override;
  bool finishedLoading() override;;
  virtual void updateDetails(Noggit::Ui::detail_infos* detail_widget) override;

  [[nodiscard]]
  AsyncObject* instance_model() const override;;

  std::vector<wmo_doodad_instance*> get_visible_doodads( math::frustum const& frustum
                                                       , float const& cull_distance
                                                       , glm::vec3 const& camera
                                                       , bool draw_hidden_models
                                                       , display_mode display
                                                       );

  std::map<uint32_t, std::vector<wmo_doodad_instance>>* get_doodads(bool draw_hidden_models);
};
