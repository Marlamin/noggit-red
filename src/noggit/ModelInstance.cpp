// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <math/bounding_box.hpp>
#include <math/frustum.hpp>
#include <glm/glm.hpp>
#include <noggit/Log.h>
#include <noggit/Misc.h> // checkinside
#include <noggit/Model.h> // Model, etc.
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/ContextObject.hpp>
#include <noggit/rendering/Primitives.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <sstream>

ModelInstance::ModelInstance(BlizzardArchive::Listfile::FileKey const& file_key
                             , Noggit::NoggitRenderContext context)
  : SceneObject(SceneObjectTypes::eMODEL, context)
  , model(file_key, context)
{
}

ModelInstance::ModelInstance(BlizzardArchive::Listfile::FileKey const& file_key
                             , ENTRY_MDDF const*d, Noggit::NoggitRenderContext context)
  : SceneObject(SceneObjectTypes::eMODEL, context)
  , model(file_key, context)
{
	uid = d->uniqueID;
	pos = glm::vec3(d->pos[0], d->pos[1], d->pos[2]);
    dir = math::degrees::vec3( math::degrees(d->rot[0])._, math::degrees(d->rot[1])._, math::degrees(d->rot[2])._);
	// scale factor - divide by 1024. blizzard devs must be on crack, why not just use a float?
	scale = d->scale / 1024.0f;
  _need_recalc_extents = true;
}


void ModelInstance::draw_box (glm::mat4x4 const& model_view
                             , glm::mat4x4 const& projection
                             , bool is_current_selection
                             )
{
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (is_current_selection)
  {
    // draw collision box
    Noggit::Rendering::Primitives::WireBox::getInstance(_context).draw ( model_view
      , projection
      , transformMatrix()
      , { 1.0f, 1.0f, 0.0f, 1.0f }
      , misc::transform_model_box_coords(model->header.collision_box_min)
      , misc::transform_model_box_coords(model->header.collision_box_max)
      );

    // draw bounding box
    Noggit::Rendering::Primitives::WireBox::getInstance(_context).draw ( model_view
      , projection
      , transformMatrix()
      , {1.0f, 1.0f, 1.0f, 1.0f}
      , misc::transform_model_box_coords(model->header.bounding_box_min)
      , misc::transform_model_box_coords(model->header.bounding_box_max)
      );

    // draw extents
    Noggit::Rendering::Primitives::WireBox::getInstance(_context).draw ( model_view
      , projection
      , glm::mat4x4(1)
      , {0.0f, 1.0f, 0.0f, 1.0f}
      , extents[0]
      , extents[1]
      );
  }
  else
  {
    const glm::vec4 color = _grouped ? glm::vec4(0.5f, 0.5f, 1.0f, 0.5f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Noggit::Rendering::Primitives::WireBox::getInstance(_context).draw ( model_view
      , projection
      , transformMatrix()
      , color
      , misc::transform_model_box_coords(model->header.bounding_box_min)
      , misc::transform_model_box_coords(model->header.bounding_box_max)
      );
  }
}

std::vector<std::tuple<int, int, int>> ModelInstance::intersect (glm::mat4x4 const& model_view
                              , math::ray const& ray
                              , selection_result* results
                              , int animtime
                              )
{  
  std::vector<std::tuple<int, int, int>> triangle_indices;
  math::ray subray (_transform_mat_inverted, ray);

  if ( !subray.intersect_bounds ( fixCoordSystem (model->header.bounding_box_min)
                                , fixCoordSystem (model->header.bounding_box_max)
                                )
     )
  {
    return triangle_indices;
  }

  for (auto&& result : model->intersect (model_view, subray, animtime))
  {
    //! \todo why is only sc important? these are relative to subray,
    //! so should be inverted by model_matrix?
    results->emplace_back (result.first * scale, this);
    triangle_indices.emplace_back(result.second);
  }
  return triangle_indices;
}


bool ModelInstance::isInFrustum(const math::frustum& frustum)
{
  if (_need_recalc_extents)
  {
    recalcExtents();
  }

  if (!frustum.intersects(extents[1], extents[0]))
    return false;

  return true;
}

bool ModelInstance::isInRenderDist(const float& cull_distance, const glm::vec3& camera, display_mode display)
{
  float dist;

  if (display == display_mode::in_3D)
  {
    dist = glm::distance(camera, pos) - model->rad * scale;
  }
  else
  {
    dist = std::abs(pos.y - camera.y) - model->rad * scale;
  }

  if (dist >= cull_distance)
  {
    return false;
  }

  if (size_cat < 1.f && dist > 300.f)
  {
    return false;
  }
  else if (size_cat < 4.f && dist > 500.f)
  {
    return false;
  }
  else if (size_cat < 25.f && dist > 1000.f)
  {
    return false;
  }

  return true;
}

void ModelInstance::recalcExtents()
{
  if (!model->finishedLoading())
  {
    _need_recalc_extents = true;
    return;
  }

  if (model->loading_failed())
  {
    extents[0] = extents[1] = pos;
    _need_recalc_extents = false;
    return;
  }

  updateTransformMatrix();

  math::aabb const relative_to_model
    ( glm::min ( model->header.collision_box_min, model->header.bounding_box_min)
    , glm::max ( model->header.collision_box_max, model->header.bounding_box_max)
    );

  //! \todo If both boxes are {inf, -inf}, or well, if any min.c > max.c,
  //! the model is bad itself. We *could* detect that case and explicitly
  //! assume {-1, 1} then, to be nice to fuckported models.

  auto corners_in_world = std::vector<glm::vec3>();
  auto transform = misc::transform_model_box_coords;
  auto points = relative_to_model.all_corners();
  for (auto& point : points)
  {
    point = transform(point);
    corners_in_world.push_back(point);
  }
 
  auto rotated_corners_in_world = std::vector<glm::vec3>();
  auto transposedMat = _transform_mat;
  for (auto const& point : corners_in_world)
  {
    rotated_corners_in_world.emplace_back(transposedMat * glm::vec4(point, 1.f));
  }

  math::aabb const bounding_of_rotated_points (rotated_corners_in_world);

  extents[0] = bounding_of_rotated_points.min;
  extents[1] = bounding_of_rotated_points.max;

  size_cat = glm::distance(bounding_of_rotated_points.max, bounding_of_rotated_points.min);

  // Calculate bounding radius
  // glm::vec3 center = (extents[0] + extents[1]) / 2.0f;
  glm::vec3 halfExtents = (extents[1] - extents[0]) / 2.0f;
  bounding_radius = glm::length(halfExtents);

  _need_recalc_extents = false;
}

void ModelInstance::ensureExtents()
{
  if (_need_recalc_extents && model->finishedLoading())
  {
    recalcExtents();
  }
}

std::array<glm::vec3, 2> const& ModelInstance::getExtents()
{
  ensureExtents();

  return extents;
}

void ModelInstance::updateDetails(Noggit::Ui::detail_infos* detail_widget)
{
  std::stringstream select_info;

  select_info << "<b>filename:</b> " << model->file_key().filepath()
    // << "<br><b>FileDataID:</b> " << model->file_key().fileDataID() // not in WOTLK
    << "<br><b>unique ID:</b> " << uid
    << "<br><b>position X/Y/Z:</b> {" << pos.x << " , " << pos.y << " , " << pos.z << "}"
    << "<br><b>rotation X/Y/Z:</b> {" << dir.x << " , " << dir.y << " , " << dir.z << "}"
    << "<br><b>scale:</b> " << scale

    << "<br><b>server position X/Y/Z: </b>{" << (ZEROPOINT - pos.z) << ", " << (ZEROPOINT - pos.x) << ", " << pos.y << "}"
    << "<br><b>server orientation:  </b>" << fabs(2 * glm::pi<float>() - glm::pi<float>() / 180.0 * (float(dir.y) < 0 ? fabs(float(dir.y)) + 180.0 : fabs(float(dir.y) - 180.0)))

    << "<br><b>textures Used:</b> " << model->header.nTextures
    << "<br><b>size category:</b><span> " << size_cat;

  for (unsigned j  = 0; j < model->header.nTextures; j++)
  {
    bool stuck = !model->_textures[j]->finishedLoading();
    bool error = model->_textures[j]->finishedLoading() && !model->_textures[j]->is_uploaded();

    select_info << "<br> ";

    if (stuck)
      select_info << "<font color=\"Orange\">";

    if (error)
      select_info << "<font color=\"Red\">";

    select_info << "<b>" << (j + 1) << ":</b> " << model->_textures[j]->file_key().stringRepr();

    if (stuck || error)
      select_info << "</font>";
  }

  select_info << "</span>";

  detail_widget->setText(select_info.str());
}

wmo_doodad_instance::wmo_doodad_instance(BlizzardArchive::Listfile::FileKey const& file_key
                                         , BlizzardArchive::ClientFile* f
                                         , Noggit::NoggitRenderContext context)
  : ModelInstance(file_key, context)
{
  float ff[4];

  f->read(ff, 12);
  pos = glm::vec3(ff[0], ff[2], -ff[1]);

  f->read(ff, 16);
  // doodad_orientation = glm::quat (-ff[0], -ff[2], ff[1], ff[3]);
  doodad_orientation = glm::quat(ff[3], -ff[0], -ff[2], ff[1]); // in GLM w is the first argument
  doodad_orientation = glm::normalize(doodad_orientation);
  dir = glm::degrees(glm::eulerAngles(doodad_orientation));

  f->read(&scale, 4);

  union
  {
    uint32_t packed;
    struct
    {
      uint8_t b, g, r, a;
    }bgra;
  } color;

  f->read(&color.packed, 4);

  light_color = glm::vec3(color.bgra.r / 255.f, color.bgra.g / 255.f, color.bgra.b / 255.f);
}

void wmo_doodad_instance::update_transform_matrix_wmo(WMOInstance* wmo)
{
  if (!model->finishedLoading() || !wmo->finishedLoading())
  {
    return;
  }  

  // world_pos = wmo->transformMatrix() * glm::vec4(pos,0);
  world_pos = wmo->transformMatrix() * glm::vec4(pos, 1.0f);

  auto m2_mat = glm::mat4x4(1.0f);

  // m2_mat = glm::translate(m2_mat, pos);
  m2_mat = m2_mat * glm::translate(glm::mat4(1.0f), pos);
  m2_mat = m2_mat * glm::toMat4(glm::inverse(doodad_orientation));
  m2_mat = glm::scale(m2_mat, glm::vec3(scale));

  _transform_mat = wmo->transformMatrix() * m2_mat;
  _transform_mat_inverted = glm::inverse(_transform_mat);

  // to compute the size category (used in culling)
  // updateTransformMatrix() is overriden to do nothing
  recalcExtents();

  _need_matrix_update = false;
}

