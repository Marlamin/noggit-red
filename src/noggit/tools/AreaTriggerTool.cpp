// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "AreaTriggerTool.hpp"

#include <math/coordinates.hpp>
#include <math/sphere.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/ui/tools/AreaTriggerEditor/AreaTriggerEditor.hpp>

#include <external/glm/gtx/matrix_decompose.hpp>
#include <external/glm/gtc/type_ptr.hpp>

#include <variant>
#include <type_traits>

size_t get_closest_hit(MapView* map_view, std::vector<Noggit::area_trigger> const& hits)
{
  size_t closest_index = 0;
  float closest_distance = std::numeric_limits<float>::max();
  auto camera_position = map_view->getCamera()->position;

  for (size_t i = 0; i < hits.size(); ++i)
  {
    auto&& hit = hits[i];

    auto distance = glm::distance(hit.position, camera_position);
    if (distance > closest_distance)
    {
      continue;
    }

    closest_distance = distance;
    closest_index = i;
  }

  return closest_index;
}

namespace Noggit
{
  AreaTriggerTool::AreaTriggerTool(MapView* mapView)
    : Tool{ mapView }
  {
    addHotkey("deleteSelection"_hash, {
        .onPress = [=] { _editor->deleteSelectedTrigger(); },
        .condition = [=] { return mapView->get_editing_mode() == editing_mode::area_trigger && !NOGGIT_CUR_ACTION; }
      });
  }

  AreaTriggerTool::~AreaTriggerTool()
  {
  }

  char const* AreaTriggerTool::name() const
  {
    return "Area Trigger";
  }

  editing_mode AreaTriggerTool::editingMode() const
  {
    return editing_mode::area_trigger;
  }

  Ui::FontNoggit::Icons AreaTriggerTool::icon() const
  {
    return Ui::FontNoggit::AREA_TRIGGER;
  }

  void AreaTriggerTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
  {
    using Ui::Tools::AreaTriggerEditor;
    _editor = new  AreaTriggerEditor{ mapView() };
    toolPanel->registerTool(this, _editor);

    QObject::connect(_editor, &AreaTriggerEditor::selectionChanged, [=](uint32_t current) { _selected_area_trigger = current; });
  }

  ToolDrawParameters AreaTriggerTool::drawParameters() const
  {
    return {};
  }

  void AreaTriggerTool::onSelected()
  {
    mapView()->enableGizmoBar();
  }

  void AreaTriggerTool::onDeselected()
  {
    mapView()->disableGizmoBar();
  }

  void AreaTriggerTool::onMouseRelease(MouseReleaseParameters const& params)
  {
    if (params.button == Qt::MouseButton::MiddleButton)
    {
      jump_to_area_trigger(params);
      return;
    }

    if (params.button == Qt::MouseButton::LeftButton && (!ImGuizmo::IsUsing() && !ImGuizmo::IsOver()))
    {
      select_area_trigger();
      return;
    }
  }

  void AreaTriggerTool::postRender()
  {
    auto const modelView = mapView()->model_view();
    auto const projection = mapView()->projection();
    math::frustum const frustum(projection * modelView);

    for (auto&& record : gAreaTriggerDB)
    {
      area_trigger areaTrigger{ record };
      if (areaTrigger.map_id != mapView()->getWorld()->getMapID())
      {
        continue;
      }

      if (areaTrigger.intersects(frustum))
      {
        glm::vec4 color{ 1.f, 1.f, 1.f, 1.f };

        if (areaTrigger.id == _selected_area_trigger)
        {
          color = glm::vec4{ 1.f, 0.f, 0.f, 1.f };
        }

        areaTrigger.draw(projection, modelView, _boxRenderer, _sphereRenderer, color);
      }
    }
  }

  void AreaTriggerTool::renderImGui(ImGuizmo::MODE mode, ImGuizmo::OPERATION operation)
  {
    auto trigger = get_selected_trigger();

    if (!trigger)
    {
      return;
    }

    glm::mat4 delta_matrix;
    if (!manipulate_gizmo(mode, operation, *trigger, delta_matrix))
    {
      return;
    }

    apply_changes(mode, operation, delta_matrix, *trigger);
  }

  void AreaTriggerTool::saveSettings()
  {
    _editor->save();

    std::vector<area_trigger> area_triggers{ gAreaTriggerDB.getRecordCount() };

    int i = 0;
    for (auto&& record : gAreaTriggerDB)
    {
      area_triggers[i++] = area_trigger{ record };
    }

    std::sort(area_triggers.begin(), area_triggers.end(), [](auto&& a, auto&& b) {
      if (a.map_id != b.map_id)
      {
        return a.map_id < b.map_id;
      }

      return a.id < b.id;
      });

    auto dbc = DBCFile::createNew("DBFilesClient\\AreaTrigger.dbc", AreaTriggerDB::FieldCount, static_cast<uint32_t>(gAreaTriggerDB.getRecordSize()));
    for (auto&& trigger : area_triggers)
    {
      auto&& record = dbc.addRecord(trigger.id);
      record.write(AreaTriggerDB::MapId, trigger.map_id);
      auto pos = math::to_server(trigger.position.x, trigger.position.y, trigger.position.z);
      record.write(AreaTriggerDB::X, pos.x);
      record.write(AreaTriggerDB::Y, pos.y);
      record.write(AreaTriggerDB::Z, pos.z);

      std::visit([&](auto&& trigger) {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(trigger)>, sphere_trigger>)
        {
          record.write(AreaTriggerDB::Radius, trigger.radius);
        }
        else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(trigger)>, box_trigger>)
        {
          auto size = trigger.extents_max - trigger.extents_min;
          record.write(AreaTriggerDB::Length, size.z);
          record.write(AreaTriggerDB::Width, size.x);
          record.write(AreaTriggerDB::Height, size.y);
          record.write(AreaTriggerDB::Orientation, trigger.orientation);
        }
        }, trigger.trigger);
    }

    dbc.save();
    gAreaTriggerDB.overwriteWith(dbc);
  }

  void AreaTriggerTool::jump_to_area_trigger(const Noggit::MouseReleaseParameters& params)
  {
    for (auto&& record : gAreaTriggerDB)
    {
      area_trigger areaTrigger{ record };
      if (areaTrigger.map_id != mapView()->getWorld()->getMapID())
      {
        continue;
      }

      if (_selected_area_trigger == std::numeric_limits<uint32_t>::max()
        && params.mod_ctrl_down)
      {
        mapView()->getCamera()->position = areaTrigger.position;
        return;
      }

      if (areaTrigger.id != _selected_area_trigger)
      {
        continue;
      }

      mapView()->getCamera()->position = areaTrigger.position;
      return;
    }
  }

  void AreaTriggerTool::select_area_trigger()
  {
    auto const ray = mapView()->intersect_ray();
    std::vector<area_trigger> hits;

    for (auto&& record : gAreaTriggerDB)
    {
      area_trigger areaTrigger{ record };
      if (areaTrigger.map_id != mapView()->getWorld()->getMapID())
      {
        continue;
      }

      if (!areaTrigger.intersects(math::frustum{ mapView()->projection() * mapView()->model_view() })
        || !areaTrigger.intersects(ray))
      {
        continue;
      }

      hits.emplace_back(areaTrigger);
    }

    if (hits.empty())
    {
      _selected_area_trigger = std::numeric_limits<uint32_t>::max();
      _editor->clearSelection();
      return;
    }

    if (hits.size() == 1)
    {
      _editor->set_selected(hits[0]);
      _selected_area_trigger = hits[0].id;
      return;
    }

    auto const closest_index = get_closest_hit(mapView(), hits);
    _editor->set_selected(hits[closest_index]);
    _selected_area_trigger = hits[closest_index].id;
  }

  std::optional<area_trigger> AreaTriggerTool::get_selected_trigger()
  {
    if (_selected_area_trigger == std::numeric_limits<uint32_t>::max())
    {
      return {};
    }

    for (auto&& record : gAreaTriggerDB)
    {
      area_trigger areaTrigger{ record };
      if (areaTrigger.id == _selected_area_trigger)
      {
        return areaTrigger;
      }
    }

    return {};
  }

  bool AreaTriggerTool::manipulate_gizmo(ImGuizmo::MODE mode, ImGuizmo::OPERATION& operation, area_trigger const& trigger, glm::mat4& out_delta_matrix)
  {
    auto const modelView = mapView()->model_view();
    auto const projection = mapView()->projection();
    auto model_view_trs = modelView;
    auto projection_trs = projection;

    ImGuizmo::SetDrawlist();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetScaleGizmoAxisLock(true);
    ImGuizmo::BeginFrame();

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    glm::mat4x4 delta_matrix = glm::mat4x4(1.0f);
    glm::mat4x4 object_matrix = glm::translate(glm::mat4x4(1.0f), trigger.position);
    float bounds[6] = {};

    if (operation == ImGuizmo::SCALE && trigger.trigger.index() == 0)
    {
      ImGuizmo::SetScaleGizmoAxisLock(true);
      auto radius = std::get<0>(trigger.trigger).radius;
      object_matrix = glm::scale(object_matrix, { radius, radius, radius });
    }
    else if (operation == ImGuizmo::SCALE)
    {
      operation = ImGuizmo::BOUNDS;

      auto box = std::get<1>(trigger.trigger);
      auto ext_max = box.extents_max;
      auto ext_min = box.extents_min;
      auto size = ext_max - ext_min;

      object_matrix = glm::rotate(object_matrix, box.orientation, { 0, 1, 0 });
      object_matrix = glm::scale(object_matrix, size);
      bounds[0] = ext_min.x / size.x;
      bounds[1] = ext_min.y / size.y;
      bounds[2] = ext_min.z / size.z;
      bounds[3] = ext_max.x / size.x;
      bounds[4] = ext_max.y / size.y;
      bounds[5] = ext_max.z / size.z;
    }

    if (!ImGuizmo::Manipulate(glm::value_ptr(model_view_trs)
      , glm::value_ptr(projection_trs)
      , operation, mode
      , glm::value_ptr(object_matrix)
      , glm::value_ptr(delta_matrix)
      , nullptr
      , operation == ImGuizmo::BOUNDS ? bounds : nullptr) && operation != ImGuizmo::BOUNDS)
    {
      return false;
    }

    if (!ImGuizmo::IsUsing())
    {
      return false;
    }

    out_delta_matrix = operation == ImGuizmo::BOUNDS ? object_matrix : delta_matrix;

    return true;
  }

  bool has_changed(const ImGuizmo::OPERATION operation, glm::vec3& new_translation, glm::quat& new_orientation, glm::vec3& new_scale)
  {
    switch (operation)
    {
    case ImGuizmo::TRANSLATE:
    {
      if (new_translation.x == 0.0f && new_translation.y == 0.0f && new_translation.z == 0.0f)
        return false;
      break;
    }
    case ImGuizmo::ROTATE:
    {
      if (new_orientation.x == -0.0f && new_orientation.y == -0.0f && new_orientation.z == -0.0f)
        return false;
      break;
    }
    case ImGuizmo::SCALE:
    {
      if (new_scale.x == 1.0f && new_scale.y == 1.0f && new_scale.z == 1.0f)
        return false;
      break;
    }
    case ImGuizmo::BOUNDS:
    {
      if (new_scale.x == 1.0f && new_scale.y == 1.0f && new_scale.z == 1.0f)
        return false;
      break;
    }
    }
    return true;
  }

  void AreaTriggerTool::apply_changes(ImGuizmo::MODE mode, ImGuizmo::OPERATION operation, glm::mat4 const& delta_matrix, area_trigger& trigger)
  {
    glm::vec3 new_scale;
    glm::quat new_orientation;
    glm::vec3 new_translation;
    glm::vec3 new_skew_;
    glm::vec4 new_perspective_;

    glm::decompose(delta_matrix,
      new_scale,
      new_orientation,
      new_translation,
      new_skew_,
      new_perspective_
    );

    if (!has_changed(operation, new_translation, new_orientation, new_scale))
    {
      return;
    }

    new_scale = glm::clamp(new_scale, 0.01f, 500.f); // These values are arbitrary. Feel free to change them

    NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eAREA_TRIGGER_TRANSFORMED, Noggit::ActionModalityControllers::eLMB);

    NOGGIT_CUR_ACTION->registerAreaTriggerTransformed(&trigger);

    if (operation == ImGuizmo::TRANSLATE)
    {
      trigger.position += new_translation;
      _editor->selection_translated(trigger.position);
    }
    if (operation == ImGuizmo::ROTATE)
    {
      std::visit([&](auto&& t)
        {
          if constexpr (std::is_same_v<std::remove_cvref_t<decltype (t)>, box_trigger>)
          {
            t.orientation += new_orientation.y;
            _editor->selection_rotated(t.orientation);
          }
        }, trigger.trigger);
    }
    if (operation == ImGuizmo::SCALE)
    {
      std::visit([&](auto&& t)
        {
          if constexpr (std::is_same_v<std::remove_cvref_t<decltype (t)>, sphere_trigger>)
          {
            t.radius = new_scale.z;
            _editor->selection_scaled(new_scale.z);
          }
          else if constexpr (std::is_same_v<std::remove_cvref_t<decltype (t)>, box_trigger>)
          {
            t.extents_min = glm::vec3{ -new_scale.x / 2, -new_scale.y / 2, -new_scale.z / 2 };
            t.extents_max = glm::vec3{ new_scale.x / 2,  new_scale.y / 2,  new_scale.z / 2 };
            _editor->selection_scaled(new_scale);
          }
        }, trigger.trigger);
    }
    if (operation == ImGuizmo::BOUNDS)
    {
      std::visit([&](auto&& t)
        {
          if constexpr (std::is_same_v<std::remove_cvref_t<decltype (t)>, box_trigger>)
          {
            trigger.position = new_translation;
            t.extents_min = glm::vec3{ -new_scale.x / 2, -new_scale.y / 2, -new_scale.z / 2 };
            t.extents_max = glm::vec3{ new_scale.x / 2,  new_scale.y / 2,  new_scale.z / 2 };
            _editor->selection_translated(new_translation);
            _editor->selection_scaled(new_scale);
          }
        }, trigger.trigger);
    }

    trigger.write_to_dbc();
  }
}
