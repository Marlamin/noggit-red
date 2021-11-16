#include "ViewportGizmo.hpp"
#include "noggit/ModelInstance.h"
#include "noggit/WMOInstance.h"
#include "noggit/ActionManager.hpp"
#include "noggit/Action.hpp"
#include "external/glm/glm.hpp"
#include <external/glm/gtx/matrix_decompose.hpp>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtc/quaternion.hpp>
#include <external/glm/gtx/string_cast.hpp>
#include <noggit/MapView.h>

#include <limits>


using namespace noggit::Red::ViewportGizmo;

ViewportGizmo::ViewportGizmo(noggit::Red::ViewportGizmo::GizmoContext gizmo_context, World* world)
: _gizmo_context(gizmo_context)
, _world(world)
{}

void ViewportGizmo::handleTransformGizmo(MapView* map_view
                                        , const std::vector<selection_type>& selection
                                        , glm::mat4x4 const& model_view
                                        , glm::mat4x4 const& projection)
{

  if (!isUsing())
  {
    _last_pivot_scale = 1.f;
  }

  GizmoInternalMode gizmo_selection_type;

  auto model_view_trs = model_view;
  auto projection_trs = projection;

  int n_selected = selection.size();

  if (!n_selected || (n_selected == 1 & selection[0].which() != eEntry_Object))
    return;

  if (n_selected == 1)
  {
    gizmo_selection_type = boost::get<selected_object_type>(selection[0])->which() == eMODEL ? GizmoInternalMode::MODEL : GizmoInternalMode::WMO;
  }
  else
  {
    gizmo_selection_type = GizmoInternalMode::MULTISELECTION;
  }

  SceneObject* obj_instance;

  ImGuizmo::SetID(_gizmo_context);

  ImGuizmo::SetDrawlist();

  ImGuizmo::SetOrthographic(false);
  ImGuizmo::SetScaleGizmoAxisLock(true);
  ImGuizmo::BeginFrame();

  ImGuiIO& io = ImGui::GetIO();
  ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

  math::matrix_4x4 delta_matrix = math::matrix_4x4(math::matrix_4x4::unit).transposed();
  math::matrix_4x4 object_matrix = {math::matrix_4x4::unit};
  math::matrix_4x4 pivot_matrix = math::matrix_4x4(math::matrix_4x4::translation,
                                                   {_multiselection_pivot.x,
                                                    _multiselection_pivot.y,
                                                    _multiselection_pivot.z}).transposed();
  float last_pivot_scale = 1.f;

  switch (gizmo_selection_type)
  {
    case MODEL:
    case WMO:
    {
      obj_instance = boost::get<selected_object_type>(selection[0]);
      obj_instance->recalcExtents();
      object_matrix = obj_instance->transformMatrixTransposed();
      ImGuizmo::Manipulate(glm::value_ptr(model_view_trs), glm::value_ptr(projection_trs), _gizmo_operation, _gizmo_mode, object_matrix, delta_matrix, nullptr);
      break;
    }
    case MULTISELECTION:
    {
      if (isUsing())
        _last_pivot_scale = ImGuizmo::GetOperationScaleLast();

      ImGuizmo::Manipulate(glm::value_ptr(model_view_trs), glm::value_ptr(projection_trs), _gizmo_operation, _gizmo_mode, pivot_matrix, delta_matrix, nullptr);
      break;
    }
  }

  if (!isUsing())
  {
    return;
  }

  noggit::ActionManager::instance()->beginAction(map_view, noggit::ActionFlags::eOBJECTS_TRANSFORMED,
                                                 noggit::ActionModalityControllers::eLMB);

  if (gizmo_selection_type == MULTISELECTION)
  {

    for (auto& selected : selection)
    {

      if (selected.which() != eEntry_Object)
        continue;

      obj_instance = boost::get<selected_object_type>(selected);
      noggit::ActionManager::instance()->getCurrentAction()->registerObjectTransformed(obj_instance);

      obj_instance->recalcExtents();
      object_matrix = math::matrix_4x4(obj_instance->transformMatrixTransposed());

      glm::mat4 glm_transform_mat = glm::make_mat4(static_cast<float*>(delta_matrix));

      glm::vec3& pos = obj_instance->pos;
      math::degrees::vec3& rotation = obj_instance->dir;
      float wmo_scale = 0.f;
      float& scale = obj_instance->which() == eMODEL ? obj_instance->scale : wmo_scale;

      glm::vec3 new_scale;
      glm::quat new_orientation;
      glm::vec3 new_translation;
      glm::vec3 new_skew_;
      glm::vec4 new_perspective_;

      glm::decompose(glm_transform_mat,
                     new_scale,
                     new_orientation,
                     new_translation,
                     new_skew_,
                     new_perspective_
      );

      new_orientation = glm::conjugate(new_orientation);

      if (_world)
        _world->updateTilesEntry(selected, model_update::remove);

      switch (_gizmo_operation)
      {
        case ImGuizmo::TRANSLATE:
        {
            pos += glm::vec3(new_translation.x, new_translation.y, new_translation.z);
          break;
        }
        case ImGuizmo::ROTATE:
        {
          auto rot_euler = glm::degrees(glm::eulerAngles(new_orientation));
          auto rot_euler_pivot = glm::eulerAngles(new_orientation);

          if (!_use_multiselection_pivot)
          {
            rotation += glm::vec3(math::degrees(rot_euler.x)._, math::degrees(rot_euler.y)._, math::degrees(rot_euler.z)._);
          }
          else
          {
            //LogDebug << rot_euler.x << " " << rot_euler.y << " " << rot_euler.z << std::endl;
            rotation.y += math::degrees(rot_euler.y)._;

            // building model matrix
            glm::mat4 model_transform = glm::make_mat4(static_cast<float*>(object_matrix));

            // only translation of pivot
            glm::mat4 transformed_pivot = glm::make_mat4(static_cast<float*>(pivot_matrix));

            // model matrix relative to translated pivot
            glm::mat4 model_transform_rel = glm::inverse(transformed_pivot) * model_transform;

            glm::mat4 gizmo_rotation = glm::mat4_cast(new_orientation);

            glm::mat4 _transformed_pivot_rot = transformed_pivot * gizmo_rotation;

            // apply transform to model matrix
            glm::mat4 result_matrix = _transformed_pivot_rot * model_transform_rel;

            glm::vec3 rot_result_scale;
            glm::quat rot_result_orientation;
            glm::vec3 rot_result_translation;
            glm::vec3 rot_result_skew_;
            glm::vec4 rot_result_perspective_;

            glm::decompose(result_matrix,
                           rot_result_scale,
                           rot_result_orientation,
                           rot_result_translation,
                           rot_result_skew_,
                           rot_result_perspective_
            );

            rot_result_orientation = glm::conjugate(rot_result_orientation);

            auto rot_result_orientation_euler = glm::degrees(glm::eulerAngles(rot_result_orientation));

            pos = {rot_result_translation.x, rot_result_translation.y, rot_result_translation.z};
            //rotation = {rot_result_orientation_euler.x, rot_result_orientation_euler.y, rot_result_orientation_euler.z};

          }

          break;
        }
        case ImGuizmo::SCALE:
        {
          scale = std::max(0.001f, scale * (new_scale.x / _last_pivot_scale));
          break;
        }
        case ImGuizmo::BOUNDS:
        {
          throw std::logic_error("Bounds are not supported by this gizmo.");
          break;
        }
      }
      obj_instance->recalcExtents();

      if (_world)
        _world->updateTilesEntry(selected, model_update::add);
    }
  }
  else
  {
    for (auto& selected : selection)
    {
      if (selected.which() != eEntry_Object)
        continue;

      obj_instance = boost::get<selected_object_type>(selected);
      noggit::ActionManager::instance()->getCurrentAction()->registerObjectTransformed(obj_instance);

      obj_instance->recalcExtents();
      object_matrix = math::matrix_4x4(obj_instance->transformMatrixTransposed());


      glm::mat4 glm_transform_mat = glm::make_mat4(static_cast<float*>(delta_matrix));

      glm::vec3& pos = obj_instance->pos;
      math::degrees::vec3& rotation = obj_instance->dir;
      float wmo_scale = 0.f;
      float& scale = obj_instance->which() == eMODEL ? obj_instance->scale : wmo_scale;

      glm::vec3 new_scale;
      glm::quat new_orientation;
      glm::vec3 new_translation;
      glm::vec3 new_skew_;
      glm::vec4 new_perspective_;

      glm::decompose(glm_transform_mat,
                     new_scale,
                     new_orientation,
                     new_translation,
                     new_skew_,
                     new_perspective_
      );

      new_orientation = glm::conjugate(new_orientation);

      if (_world)
        _world->updateTilesEntry(selected, model_update::remove);

      switch (_gizmo_operation)
      {

        case ImGuizmo::TRANSLATE:
        {
            pos += glm::vec3(new_translation.x, new_translation.y, new_translation.z);
          break;
        }
        case ImGuizmo::ROTATE:
        {
          auto rot_euler = glm::eulerAngles(new_orientation) * 57.2957795f;
          rotation += glm::vec3(math::degrees(rot_euler.x)._, math::degrees(rot_euler.y)._, math::degrees(rot_euler.z)._);
          break;
        }
        case ImGuizmo::SCALE:
        {
          scale = std::max(0.001f, new_scale.x);
          break;
        }
        case ImGuizmo::BOUNDS:
        {
          throw std::logic_error("Bounds are not supported by this gizmo.");
        }
      }
      obj_instance->recalcExtents();

      if (_world)
        _world->updateTilesEntry(selected, model_update::add);
    }
  }
}

