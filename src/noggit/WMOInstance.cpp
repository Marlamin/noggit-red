// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>
#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/Misc.h> // checkinside
#include <noggit/ModelInstance.h>
#include <noggit/WMO.h> // WMO
#include <noggit/MapTile.h>
#include <noggit/WMOInstance.h>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>

WMOInstance::WMOInstance(std::string const& filename, ENTRY_MODF const* d, noggit::NoggitRenderContext context)
  : SceneObject(SceneObjectTypes::eWMO, context, filename)
  , wmo(filename, context)
  , mFlags(d->flags)
  , mUnknown(d->unknown), mNameset(d->nameSet)
  , _doodadset(d->doodadSet)
{
  pos = glm::vec3(d->pos[0], d->pos[1], d->pos[2]);
  dir = math::degrees::vec3{math::degrees(d->rot[0])._, math::degrees(d->rot[1])._, math::degrees(d->rot[2])._ };

  uid = d->uniqueID;

  extents[0] = glm::vec3(d->extents[0][0], d->extents[0][1], d->extents[0][2]);
  extents[1] = glm::vec3(d->extents[1][0], d->extents[1][1], d->extents[1][2]);

  updateTransformMatrix();
  change_doodadset(_doodadset);
}

WMOInstance::WMOInstance(std::string const& filename, noggit::NoggitRenderContext context)
  : SceneObject(SceneObjectTypes::eWMO, context, filename)
  , wmo(filename, context)
  , mFlags(0)
  , mUnknown(0)
  , mNameset(0)
  , _doodadset(0)
{
  change_doodadset(_doodadset);
  pos = glm::vec3(0.0f, 0.0f, 0.0f);
  dir = math::degrees::vec3(math::degrees(0)._, math::degrees(0)._, math::degrees(0)._);
  uid = 0;
  _context = context;
}


void WMOInstance::draw ( opengl::scoped::use_program& wmo_shader
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
                                          return type.type() == typeid(selected_object_type)
                                          && boost::get<selected_object_type>(type)->which() == SceneObjectTypes::eWMO
                                          && static_cast<WMOInstance*>(boost::get<selected_object_type>(type))->uid == id;
                                        }) != selection.end();

  {
    unsigned region_visible = 0;

    for (auto& tile : getTiles())
    {
      if (tile->objects_frustum_cull_test && !tile->tile_occluded)
      {
        region_visible = tile->objects_frustum_cull_test;

        if (tile->objects_frustum_cull_test > 1)
          break;
      }
    }

    if (!region_visible || (region_visible <= 1 && !frustum.intersects(extents[1], extents[0])))
    {
      return;
    }

    wmo_shader.uniform("transform", _transform_mat_transposed);

    wmo->draw ( wmo_shader
              , model_view
              , projection
              , _transform_mat
              , _transform_mat_transposed
              , is_selected
              , frustum
              , cull_distance
              , camera
              , draw_doodads
              , draw_fog
              , animtime
              , world_has_skies
              , display
              );
  }

  if (force_box || is_selected)
  {
    //gl.enable(GL_BLEND);
    //gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::vec4 color = force_box ? glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
        : glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    opengl::primitives::wire_box::getInstance(_context).draw(model_view
       , projection
       , math::matrix_4x4(math::matrix_4x4::unit)
       , color
       , extents[0]
       , extents[1]);
  }
}

void WMOInstance::intersect (math::ray const& ray, selection_result* results)
{
  if (!ray.intersect_bounds (extents[0], extents[1]))
  {
    return;
  }

  math::ray subray(_transform_mat_inverted, ray);

  for (auto&& result : wmo->intersect(subray))
  {
    results->emplace_back (result, this);
  }
}

void WMOInstance::ensureExtents()
{
  recalcExtents();
  // TODO: optimize
}

void WMOInstance::updateDetails(noggit::ui::detail_infos* detail_widget)
{
  std::stringstream select_info;

  select_info << "<b>filename: </b>" << wmo->filename
    << "<br><b>unique ID: </b>" << uid
    << "<br><b>position X/Y/Z: </b>{" << pos.x << ", " << pos.y << ", " << pos.z << "}"
    << "<br><b>rotation X/Y/Z: </b>{" << dir.x << ", " << dir.y << ", " << dir.z << "}"
    << "<br><b>doodad set: </b>" << doodadset()
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

    select_info  << "<b>" << (j + 1) << ":</b> " << wmo->textures[j]->filename;

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

  auto&& root_points = _transform_mat * math::aabb(wmo_min, wmo_max).all_corners();

  points.insert(points.end(), root_points.begin(), root_points.end());

  for (int i = 0; i < (int)wmo->groups.size(); ++i)
  {
    auto const& group = wmo->groups[i];
    auto&& group_points = _transform_mat
      * math::aabb( group.BoundingBoxMin // no need to use misc::transform_model_box_coords
                  , group.BoundingBoxMax // they are already in world coord (see group ctor)
                  ).all_corners();

    points.insert(points.end(), group_points.begin(), group_points.end());

    if (group.has_skybox())
    {
      math::aabb const group_aabb(group_points);

      group_extents[i] = {group_aabb.min, group_aabb.max};
    }
  }

  math::aabb const wmo_aabb(points);

  extents[0] = wmo_aabb.min;
  extents[1] = wmo_aabb.max;
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
        if (doodad.need_matrix_update())
        {
          doodad.update_transform_matrix_wmo(this);
        }
      }
    }
  }

  return &_doodads_per_group;
}
