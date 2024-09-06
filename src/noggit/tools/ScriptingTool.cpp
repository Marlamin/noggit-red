// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ScriptingTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_settings.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

namespace Noggit
{
    ScriptingTool::ScriptingTool(MapView* mapView)
        : Tool{ mapView }
    {
    }

    ScriptingTool::~ScriptingTool()
    {
        delete _scriptingTool;
    }

    char const* ScriptingTool::name() const
    {
        return "Scripting";
    }

    editing_mode ScriptingTool::editingMode() const
    {
        return editing_mode::scripting;
    }

    Ui::FontNoggit::Icons ScriptingTool::icon() const
    {
        return Ui::FontNoggit::INFO;
    }

    void ScriptingTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _scriptingTool = new Noggit::Scripting::scripting_tool(mapView(), mapView(), mapView()->settings());
        toolPanel->registerTool(name(), _scriptingTool);
    }

    ToolDrawParameters ScriptingTool::drawParameters() const
    {
        return
        {
            .radius = _scriptingTool->get_settings()->brushRadius(),
            .inner_radius = _scriptingTool->get_settings()->innerRadius(),
        };
    }

    void ScriptingTool::onTick(float deltaTime, TickParameters const& params)
    {
        auto world = mapView()->getWorld();

        auto currentSelection = world->current_selection();
        if (world->has_selection())
        {
            for (auto& selection : currentSelection)
            {
                if (selection.index() == eEntry_MapChunk)
                {
                    _scriptingTool->sendBrushEvent(mapView()->cursorPosition(), 7.5f * deltaTime);
                }
            }
        }
    }
}