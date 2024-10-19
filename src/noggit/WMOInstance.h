// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/SceneObject.hpp>
#include <math/ray.hpp>
#include <noggit/WMO.h>
#include <noggit/ContextObject.hpp>

#include <cstdint>
#include <set>

struct ENTRY_MODF;

class WMOInstance : public SceneObject
{
public:
  scoped_wmo_reference wmo;

  uint16_t mFlags;
  uint16_t mNameset; 

  uint16_t doodadset() const { return _doodadset; }
  void change_doodadset(uint16_t doodad_set);

  [[nodiscard]]
  std::map<int, std::pair<glm::vec3, glm::vec3>> const& getGroupExtents() { _update_group_extents = true; ensureExtents(); return group_extents; }

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

  WMOInstance (WMOInstance&& other)
    : SceneObject(other._type, other._context)
    , wmo (std::move (other.wmo))
    , group_extents(other.group_extents)
    , mFlags (other.mFlags)
    , mNameset (other.mNameset)
    , _doodadset (other._doodadset)
    , _doodads_per_group(other._doodads_per_group)
    , _need_doodadset_update(other._need_doodadset_update)
    , _need_recalc_extents(other._need_recalc_extents)
  {
    std::swap (extents, other.extents);
    pos = other.pos;
    scale = other.scale;
    dir = other.dir;
    _context = other._context;
    uid = other.uid;

    _transform_mat = other._transform_mat;
    _transform_mat_inverted = other._transform_mat_inverted;
  }

  WMOInstance& operator= (WMOInstance&& other)
  {
    std::swap(wmo, other.wmo);
    std::swap(pos, other.pos);
    std::swap(extents, other.extents);
    std::swap(group_extents, other.group_extents);
    std::swap(dir, other.dir);
    std::swap(uid, other.uid);
    std::swap(scale, other.scale);
    std::swap(mFlags, other.mFlags);
    std::swap(mNameset, other.mNameset);
    std::swap(_doodadset, other._doodadset);
    std::swap(_doodads_per_group, other._doodads_per_group);
    std::swap(_need_doodadset_update, other._need_doodadset_update);
    std::swap(_transform_mat, other._transform_mat);
    std::swap(_transform_mat_inverted, other._transform_mat_inverted);
    std::swap(_context, other._context);
    std::swap(_need_recalc_extents, other._need_recalc_extents);
    return *this;
  }

  void draw ( OpenGL::Scoped::use_program& wmo_shader
            , const glm::mat4x4 const& model_view
            , const glm::mat4x4 const& projection
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
  inline bool extentsDirty() const { return _need_recalc_extents || !wmo->finishedLoading(); };
  void recalcExtents() override;
  void change_nameset(uint16_t name_set);
  void ensureExtents() override;
  bool finishedLoading() override { return wmo->finishedLoading(); };
  virtual void updateDetails(Noggit::Ui::detail_infos* detail_widget) override;

  [[nodiscard]]
  AsyncObject* instance_model() const override { return wmo.get(); };

  std::vector<wmo_doodad_instance*> get_visible_doodads( math::frustum const& frustum
                                                       , float const& cull_distance
                                                       , glm::vec3 const& camera
                                                       , bool draw_hidden_models
                                                       , display_mode display
                                                       );

  std::map<uint32_t, std::vector<wmo_doodad_instance>>* get_doodads(bool draw_hidden_models);
};
