// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/area_trigger.hpp>
#include <noggit/Tool.hpp>
#include <noggit/rendering/Primitives.hpp>

#include <map>

namespace Noggit
{
  namespace Ui
  {
    class hole_tool;

    namespace Tools
    {
      class AreaTriggerEditor;
    }
  }

  class AreaTriggerTool final : public Tool
  {
  public:
    AreaTriggerTool(MapView* mapView);
    ~AreaTriggerTool();

    [[nodiscard]]
    virtual char const* name() const override;

    [[nodiscard]]
    virtual editing_mode editingMode() const override;

    [[nodiscard]]
    virtual Ui::FontNoggit::Icons icon() const override;

    void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

    [[nodiscard]]
    ToolDrawParameters drawParameters() const override;

    void onSelected() override;

    void onDeselected() override;

    void onMouseRelease(MouseReleaseParameters const& params) override;

    void postRender() override;

    void renderImGui(ImGuizmo::MODE mode, ImGuizmo::OPERATION operation) override;

    void saveSettings() override;

  private:
    Ui::Tools::AreaTriggerEditor* _editor = nullptr;
    Noggit::Rendering::Primitives::WireBox _boxRenderer;
    Noggit::Rendering::Primitives::Sphere _sphereRenderer;

    uint32_t _selected_area_trigger = std::numeric_limits<uint32_t>::max();

    void jump_to_area_trigger(const Noggit::MouseReleaseParameters& params);

    void select_area_trigger();

    std::optional<area_trigger> get_selected_trigger();

    bool manipulate_gizmo(ImGuizmo::MODE mode, ImGuizmo::OPERATION& operation, area_trigger const& trigger, glm::mat4& out_delta_matrix);

    void apply_changes(ImGuizmo::MODE mode, ImGuizmo::OPERATION operation, glm::mat4 const& delta_matrix, area_trigger& trigger);
  };
}
