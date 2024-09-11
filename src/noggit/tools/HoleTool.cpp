// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "HoleTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/hole_tool.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

namespace Noggit
{
    HoleTool::HoleTool(MapView* mapView)
        : Tool{ mapView }
    {
        addHotkey("unsetAdtHole"_hash, Hotkey{
            .onPress = [=] {
                NOGGIT_ACTION_MGR->beginAction(mapView, Noggit::ActionFlags::eCHUNKS_HOLES);
                mapView->getWorld()->setHoleADT(mapView->getCamera()->position, false);
                NOGGIT_ACTION_MGR->endAction();
            },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::holes && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("setAdtHole"_hash, Hotkey{
            .onPress = [=] {
                NOGGIT_ACTION_MGR->beginAction(mapView, Noggit::ActionFlags::eCHUNKS_HOLES);
                mapView->getWorld()->setHoleADT(mapView->getCamera()->position, true);
                NOGGIT_ACTION_MGR->endAction();
            },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::holes && !NOGGIT_CUR_ACTION; },
            });
    }

    HoleTool::~HoleTool()
    {
        delete _holeTool;
    }

    char const* HoleTool::name() const
    {
        return "Hole Cutter";
    }

    editing_mode HoleTool::editingMode() const
    {
        return editing_mode::holes;
    }

    Ui::FontNoggit::Icons HoleTool::icon() const
    {
        return Ui::FontNoggit::TOOL_HOLE_CUTTER;
    }

    void HoleTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _holeTool = new Noggit::Ui::hole_tool(mapView());
        toolPanel->registerTool(this, _holeTool);
    }

    ToolDrawParameters HoleTool::drawParameters() const
    {
        return
        {
            .radius = _holeTool->brushRadius(),
        };
    }

    void HoleTool::onSelected()
    {
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_lines = true;
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_hole_lines = true;
    }

    void HoleTool::onDeselected()
    {
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_lines = mapView()->drawAdtGrid();
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_hole_lines = mapView()->drawHoleGrid();
    }

    void HoleTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (!mapView()->getWorld()->has_selection() || !params.left_mouse)
        {
            return;
        }

        auto mv = mapView();
        // no undermap check here, else it's impossible to remove holes
        if (params.mod_shift_down)
        {
            NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_HOLES,
                Noggit::ActionModalityControllers::eSHIFT
                | Noggit::ActionModalityControllers::eLMB);
            mv->getWorld()->setHole(mv->cursorPosition(), _holeTool->brushRadius(), params.mod_alt_down, false);
        }
        else if (params.mod_ctrl_down && !params.underMap)
        {
            NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_HOLES,
                Noggit::ActionModalityControllers::eCTRL
                | Noggit::ActionModalityControllers::eLMB);
            mv->getWorld()->setHole(mv->cursorPosition(), _holeTool->brushRadius(), params.mod_alt_down, true);
        }
    }

    void HoleTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.left_mouse && params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
        {
            _holeTool->changeRadius(params.relative_movement.dx() / XSENS);
        }
    }
}
