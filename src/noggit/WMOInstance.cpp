// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/MapTile.h>
#include <noggit/ModelInstance.h>
#include <noggit/rendering/Primitives.hpp>
#include <noggit/scoped_blp_texture_reference.hpp>
#include <noggit/TextureManager.h>
#include <noggit/WMO.h> // WMO
#include <noggit/WMOInstance.h>

#include <opengl/shader.hpp>

#include <math/bounding_box.hpp>
#include <math/frustum.hpp>
#include <math/ray.hpp>

#include <sstream>

#include <QtCore/QSettings>

WMOInstance::WMOInstance(BlizzardArchive::Listfile::FileKey const& file_key, ENTRY_MODF const* d, Noggit::NoggitRenderContext context)
  : SceneObject(SceneObjectTypes::eWMO, context)
  , wmo(file_key, context)
  , mFlags(d->flags)
  , mNameset(d->nameSet)
  , _doodadset(d->doodadSet)
{
  pos = glm::vec3(d->pos[0], d->pos[1], d->pos[2]);
  dir = math::degrees::vec3{math::degrees(d->rot[0])._, math::degrees(d->rot[1])._, math::degrees(d->rot[2])._ };

  uid = d->uniqueID;

  bool modern_features = Noggit::Application::NoggitApplication::instance()->getConfiguration()->modern_features;

  if (modern_features)
  {
      scale = static_cast<float>(d->scale) / 1024.0f;
  }
  else
  {
      scale = 1.0f;
  }

  extents[0] = glm::vec3(d->extents[0][0], d->extents[0][1], d->extents[0][2]);
  extents[1] = glm::vec3(d->extents[1][0], d->extents[1][1], d->extents[1][2]);

  _need_recalc_extents = true;
  updateTransformMatrix();
  change_doodadset(_doodadset);
}

WMOInstance::WMOInstance(BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context)
  : SceneObject(SceneObjectTypes::eWMO, context)
  , wmo(file_key, context)
  , mFlags(0)
  , mNameset(0)
  , _doodadset(0)
{
  change_doodadset(_doodadset);
  pos = glm::vec3(0.0f, 0.0f, 0.0f);
  dir = math::degrees::vec3(math::degrees(0)._, math::degrees(0)._, math::degrees(0)._);
  uid = 0;
  _context = context;

  _need_recalc_extents = true;
  updateTransformMatrix();
}

WMOInstance::WMOInstance(WMOInstance&& other) noexcept
  : SceneObject(other._type, other._context)
  , wmo(std::move(other.wmo))
  , group_extents(other.group_extents)
  , mFlags(other.mFlags)
  , mNameset(other.mNameset)
  , _doodadset(other._doodadset)
  , _doodads_per_group(other._doodads_per_group)
  , _need_doodadset_update(other._need_doodadset_update)
  , _need_recalc_extents(other._need_recalc_extents)
{
  std::swap(extents, other.extents);
  pos = other.pos;
  scale = other.scale;
  dir = other.dir;
  _context = other._context;
  uid = other.uid;

  _transform_mat = other._transform_mat;
  _transform_mat_inverted = other._transform_mat_inverted;
}

WMOInstance& WMOInstance::operator= (WMOInstance&& other) noexcept
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

void WMOInstance::draw ( OpenGL::Scoped::use_program& wmo_shader
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
                       )
{
  if (!wmo->finishedLoading() || wmo->loading_failed())
  {
    return;
  }

  ensureExtents();

  {
    unsigned region_visible = 0;

    if (!no_cull)
    {
      for (auto& tile : getTiles())
      {
        if (tile->renderer()->objectsFrustumCullTest() && !tile->renderer()->isOccluded())
        {
          region_visible = tile->renderer()->objectsFrustumCullTest();

          if (tile->renderer()->objectsFrustumCullTest() > 1)
            break;
        }
      }
    }

    if (!no_cull && (!region_visible || (region_visible <= 1 && !frustum.intersects(extents[1], extents[0]))))
    {
      return;
    }

    wmo_shader.uniform("transform", _transform_mat);

    wmo->renderer()->draw( wmo_shader
              , model_view
              , projection
              , _transform_mat
              , is_selected || _grouped
              , frustum
              , cull_distance
              , camera
              , draw_doodads
              , draw_fog
              , animtime
              , world_has_skies
              , display
              , !draw_exterior
              , render_group_bounds
              , _grouped
              );
  }

  // axis aligned bounding box (extents)
  if (render_selection_aabb && (force_box || is_selected) && !_grouped)
  {
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::vec4 color = force_box || _grouped ? glm::vec4(0.5f, 0.5f, 1.0f, 0.5f) // light purple-blue
        : glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // green

    Noggit::Rendering::Primitives::WireBox::getInstance(_context).draw(model_view
       , projection
       , glm::mat4x4(glm::mat4x4(1))
       , color
       , extents[0]
       , extents[1]);
  }
}

void WMOInstance::intersect (math::ray const& ray, selection_result* results, bool do_exterior)
{
  if (!finishedLoading() || wmo->loading_failed())
    return;

  ensureExtents();

  if (!ray.intersect_bounds (extents[0], extents[1]))
  {
    return;
  }

  math::ray subray(_transform_mat_inverted, ray);

  for (auto&& result : wmo->intersect(subray, do_exterior))
  {
    results->emplace_back (result, this);
  }
}

std::array<glm::vec3, 2> const& WMOInstance::getExtents()
{
  ensureExtents();

  return extents;
}

std::array<glm::vec3, 2> const& WMOInstance::getLocalExtents() const
{

  return { wmo->extents[0], wmo->extents[1] };
}

std::array<glm::vec3, 8> WMOInstance::getBoundingBox()
{
  // auto extents = getExtents();
  if (_need_recalc_extents)
    updateTransformMatrix();

  // note, doesn't include group bounds like recalcExtents(), this just trusts blizzard.

  math::aabb const relative_to_model(wmo->extents[0], wmo->extents[1]);

  return relative_to_model.rotated_corners(_transform_mat, true);
}

// not axis aligned
bool WMOInstance::extentsDirty() const
{
  return _need_recalc_extents || !wmo->finishedLoading();
}

void WMOInstance::ensureExtents()
{
  if ( (_need_recalc_extents || _update_group_extents) && wmo->finishedLoading())
  {
    recalcExtents();
  }
}

bool WMOInstance::finishedLoading()
{
  return wmo->finishedLoading();
}

void WMOInstance::updateDetails(Noggit::Ui::detail_infos* detail_widget)
{
  std::stringstream select_info;

  select_info << "<b>filename: </b>" << wmo->file_key().filepath()
    // << "<br><b>FileDataID: </b>" << wmo->file_key().fileDataID() not in wrath
    << "<br><b>unique ID: </b>" << uid
    << "<br><b>position X/Y/Z: </b>{" << pos.x << ", " << pos.y << ", " << pos.z << "}"
    << "<br><b>rotation X/Y/Z: </b>{" << dir.x << ", " << dir.y << ", " << dir.z << "}"
    << "<br><b>WMO Id: </b>" << wmo->WmoId
    << "<br><b>doodad set: </b>" << doodadset()
    << "<br><b>name set: </b>" << mNameset

    << "<br><b>server position X/Y/Z: </b>{" << (ZEROPOINT - pos.z) << ", " << (ZEROPOINT - pos.x) << ", " << pos.y << "}"
    << "<br><b>server orientation:  </b>" << fabs(2 * glm::pi<float>() - glm::pi<float>() / 180.0 * (float(dir.y) < 0 ? fabs(float(dir.y)) + 180.0 : fabs(float(dir.y) - 180.0)))

    << "<br><b>textures used: </b>" << wmo->textures.size()
    << "<span>";

  for (unsigned j = 0; j < wmo->textures.size(); j++)
  {
    bool stuck = !wmo->textures[j]->finishedLoading();
    bool error = wmo->textures[j]->finishedLoading() && !wmo->textures[j]->is_uploaded();

    select_info << "<br> ";

    if (stuck)
      select_info << "<font color=\"Orange\">";

    if (error)
      select_info << "<font color=\"Red\">";

    select_info  << "<b>" << (j + 1) << ":</b> " << wmo->textures[j]->file_key().stringRepr();

    if (stuck || error)
      select_info << "</font>";
  }

  select_info << "<br></span>";

  detail_widget->setText(select_info.str());
}

[[nodiscard]]
AsyncObject* WMOInstance::instance_model() const
{
  return wmo.get();
}

void WMOInstance::recalcExtents()
{
  // keep the old extents since they are saved in the adt
  if (!wmo->finishedLoading())
  {
      _need_recalc_extents = true;
      return;
  }

  if (wmo->loading_failed())
  {
      extents[0] = extents[1] = pos;
      _need_recalc_extents = false;
      return;
  }

  updateTransformMatrix();
  update_doodads();

  std::vector<glm::vec3> points;

  std::array<glm::vec3, 8> const adjustedPoints = math::aabb(wmo->extents[0], wmo->extents[1]).rotated_corners(_transform_mat, true);

  points.insert(points.end(), adjustedPoints.begin(), adjustedPoints.end());

  for (int i = 0; i < (int)wmo->groups.size(); ++i)
  {
    auto const& group = wmo->groups[i];

    const auto&& group_points = math::aabb(group.BoundingBoxMin, group.BoundingBoxMax).all_corners();
    std::array<glm::vec3, 8> adjustedGroupPoints;

    for (int i = 0; i < 8; ++i)
    {
      adjustedGroupPoints[i] = _transform_mat * glm::vec4(group_points[i], 1.f);
    }

    points.insert(points.end(), adjustedGroupPoints.begin(), adjustedGroupPoints.end());

    if (group.has_skybox() || _update_group_extents)
    {
      math::aabb const group_aabb(std::vector<glm::vec3>(adjustedGroupPoints.begin()
                                  , adjustedGroupPoints.end()));

      group_extents[i] = {group_aabb.min, group_aabb.max};
      _update_group_extents = false;
    }
  }

  math::aabb const wmo_aabb(points);

  extents[0] = wmo_aabb.min;
  extents[1] = wmo_aabb.max;

  bounding_radius = glm::distance(wmo->extents[1], wmo->extents[0]) * scale / 2.0f;

  _need_recalc_extents = false;
}

void WMOInstance::change_nameset(uint16_t name_set)
{
    mNameset = name_set;
}

uint16_t WMOInstance::doodadset() const
{
  return _doodadset;
}

void WMOInstance::change_doodadset(uint16_t doodad_set)
{
  if (!wmo->finishedLoading())
  {
    _need_doodadset_update = true;
    return;
  }

  // don't set an invalid doodad set
  if (doodad_set >= wmo->doodadsets.size())
  {
    return;
  }

  _doodadset = doodad_set;
  _doodads_per_group = wmo->doodads_per_group(_doodadset);
  _need_doodadset_update = false;

  ensureExtents();
  update_doodads();
}

[[nodiscard]]
std::map<int, std::pair<glm::vec3, glm::vec3>> const& WMOInstance::getGroupExtents()
{
  _update_group_extents = true;
  ensureExtents();
  return group_extents;
}

void WMOInstance::update_doodads()
{
  for (auto& group_doodads : _doodads_per_group)
  {
    for (auto& doodad : group_doodads.second)
    {
      // if (!doodad.need_matrix_update())
      //   continue;

      doodad.update_transform_matrix_wmo(this);
    }
  }
}

std::vector<wmo_doodad_instance*> WMOInstance::get_visible_doodads
  ( math::frustum const& frustum
  , float const& cull_distance
  , glm::vec3 const& camera
  , bool draw_hidden_models
  , display_mode display
  )
{
  std::vector<wmo_doodad_instance*> doodads;

  if (!wmo->finishedLoading() || wmo->loading_failed())
  {
    return doodads;
  }

  if (_need_doodadset_update)
  {
    change_doodadset(_doodadset);
  }

  if (!wmo->is_hidden() || draw_hidden_models)
  {
    for (int i = 0; i < wmo->groups.size(); ++i)
    {
      if (wmo->groups[i].is_visible(_transform_mat, frustum, cull_distance, camera, display))
      {
        for (auto& doodad : _doodads_per_group[i])
        {
          if (doodad.need_matrix_update())
          {
            doodad.update_transform_matrix_wmo(this);
          }

          doodads.push_back(&doodad);
        }
      }
    }
  } 

  return doodads;
}

std::map<uint32_t, std::vector<wmo_doodad_instance>>* WMOInstance::get_doodads(bool draw_hidden_models)
{

  if (!wmo->finishedLoading() || wmo->loading_failed())
  {
    return nullptr;
  }

  if (_need_doodadset_update)
  {
    change_doodadset(_doodadset);
  }

  if (wmo->is_hidden() && !draw_hidden_models)
  {
    return nullptr;
  }
  else
  {
    for (int i = 0; i < wmo->groups.size(); ++i)
    {
      for (auto& doodad : _doodads_per_group[i])
      {
        if (doodad.finishedLoading() && doodad.need_matrix_update())
        {
          doodad.update_transform_matrix_wmo(this);
        }
      }
    }
  }

  return &_doodads_per_group;
}
