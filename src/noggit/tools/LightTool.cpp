// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LightTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/tools/LightEditor/LightEditor.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

namespace Noggit
{
    LightTool::LightTool(MapView* mapView)
        : Tool{ mapView }
    {
    }

    LightTool::~LightTool()
    {
        delete _lightEditor;
    }

    char const* LightTool::name() const
    {
        return "Light Editor";
    }

    editing_mode LightTool::editingMode() const
    {
        return editing_mode::light;
    }

    Ui::FontNoggit::Icons LightTool::icon() const
    {
        return Ui::FontNoggit::TOOL_LIGHT;
    }

    void LightTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _lightEditor = new Noggit::Ui::Tools::LightEditor(mapView(), mapView());
        toolPanel->registerTool(name(), _lightEditor);
    }

    void LightTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (mapView()->timeSpeed() > 0.0f)
            _lightEditor->UpdateWorldTime();
    }
}