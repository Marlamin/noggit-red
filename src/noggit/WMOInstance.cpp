// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>
#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/Misc.h> // checkinside
#include <noggit/ModelInstance.h>
#include <noggit/WMO.h> // WMO
#include <noggit/MapTile.h>
#include <noggit/WMOInstance.h>
#include <noggit/rendering/Primitives.hpp>
#include <opengl/scoped.hpp>

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

  QSettings settings;
  bool modern_features = settings.value("modern_features", false).toBool();

  if (modern_features) {
      scale = static_cast<float>(d->scale) / 1024.0f;
  }
  else {
      scale = 1.0f;
  }

  extents[0] = glm::vec3(d->extents[0][0], d->extents[0][1], d->extents[0][2]);
  extents[1] = glm::vec3(d->extents[1][0], d->extents[1][1], d->extents[1][2]);

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

  updateTransformMatrix();
}


void WMOInstance::draw ( OpenGL::Scoped::use_program& wmo_shader
                       , glm::mat4x4 const& model_view
                       , glm::mat4x4 const& projection
                       , math::frustum const& frustum
                       , const float& cull_distance
                       , const glm::vec3& camera
                       , bool force_box
                       , bool draw_doodads
                       , bool draw_fog
                       , std::vector<selection_type> selection
                       , int animtime
                       , bool world_has_skies
                       , display_mode display
                       , bool no_cull
                       , bool draw_exterior
                       )
{
  if (!wmo->finishedLoading() || wmo->loading_failed())
  {
    return;
  }

  const uint id = this->uid;
  bool const is_selected = selection.size() > 0 &&
                           std::find_if(selection.begin(), selection.end(),
                                        [id](selection_type type)
                                        {
                                          return var_type(type) == typeid(selected_object_type)
                                          && std::get<selected_object_type>(type)->which() == SceneObjectTypes::eWMO
                                          && static_cast<WMOInstance*>(std::get<selected_object_type>(type))->uid == id;
                                        }) != selection.end();

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
              , is_selected && !_grouped
              , frustum
              , cull_distance
              , camera
              , draw_doodads
              , draw_fog
              , animtime
              , world_has_skies
              , display
              , !draw_exterior
              );
  }

  if (force_box || is_selected)
  {
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::vec4 color = force_box || _grouped ? glm::vec4(0.5f, 0.5f, 1.0f, 0.5f)
        : glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

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

void WMOInstance::ensureExtents()
{
  recalcExtents();
  // TODO: optimize
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

void WMOInstance::recalcExtents()
{
  // todo: keep track of whether the extents need to be recalculated or not
  // keep the old extents since they are saved in the adt
  if (wmo->loading_failed() || !wmo->finishedLoading())
  {
    return;
  }

  updateTransformMatrix();
  update_doodads();

  std::vector<glm::vec3> points;

  glm::vec3 wmo_min(misc::transform_model_box_coords(wmo->extents[0]));
  glm::vec3 wmo_max(misc::transform_model_box_coords(wmo->extents[1]));

  auto&& root_points = math::aabb(wmo_min, wmo_max).all_corners(); 
  auto adjustedPoints = std::vector<glm::vec3>();

  for (auto const& point : root_points)
  {
    adjustedPoints.push_back(_transform_mat * glm::vec4(point, 1.f));
  }

  points.insert(points.end(), adjustedPoints.begin(), adjustedPoints.end());

  for (int i = 0; i < (int)wmo->groups.size(); ++i)
  {
    auto const& group = wmo->groups[i];

    auto&& group_points = math::aabb(group.BoundingBoxMin, group.BoundingBoxMax).all_corners();
    auto adjustedGroupPoints = std::vector<glm::vec3>();

    for (auto const& point : group_points)
    {
      adjustedGroupPoints.push_back(_transform_mat * glm::vec4(point, 1.f));
    }


    points.insert(points.end(), adjustedGroupPoints.begin(), adjustedGroupPoints.end());

    if (group.has_skybox() || _update_group_extents)
    {
      math::aabb const group_aabb(adjustedGroupPoints);

      group_extents[i] = {group_aabb.min, group_aabb.max};
    }
  }
  _update_group_extents = false;

  math::aabb const wmo_aabb(points);

  extents[0] = wmo_aabb.min;
  extents[1] = wmo_aabb.max;
}

void WMOInstance::change_nameset(uint16_t name_set)
{
    mNameset = name_set;
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

  update_doodads();
}

void WMOInstance::update_doodads()
{
  for (auto& group_doodads : _doodads_per_group)
  {
    for (auto& doodad : group_doodads.second)
    {
      if (!doodad.need_matrix_update())
        continue;

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
