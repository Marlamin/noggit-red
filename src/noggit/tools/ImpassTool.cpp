// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImpassTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

namespace Noggit
{
    ImpassTool::ImpassTool(MapView* mapView)
        : Tool{ mapView }
    {
    }

    char const* ImpassTool::name() const
    {
        return "Impass Designator";
    }

    editing_mode ImpassTool::editingMode() const
    {
        return editing_mode::impass;
    }

    Ui::FontNoggit::Icons ImpassTool::icon() const
    {
        return Ui::FontNoggit::TOOL_IMPASS_DESIGNATOR;
    }

    void ImpassTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        // Dummy, because the toolbar requires a widget for every tool
        toolPanel->registerTool(this, new  QWidget{mapView()});
    }

    void ImpassTool::onSelected()
    {
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_impass_overlay = true;
    }

    void ImpassTool::onDeselected()
    {
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_impass_overlay = false;
    }

    void ImpassTool::onTick(float deltaTime, TickParameters const& params)
    {
        mapView()->doSelection(true);
        if (!mapView()->getWorld()->has_selection() || params.underMap || !params.left_mouse)
        {
            return;
        }

        // todo: replace this  -- why and with what? 
        if (params.mod_shift_down)
        {
            NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_FLAGS,
                Noggit::ActionModalityControllers::eSHIFT
                | Noggit::ActionModalityControllers::eLMB);
            mapView()->getWorld()->mapIndex.setFlag(true, mapView()->cursorPosition(), 0x2);
        }
        else if (params.mod_ctrl_down)
        {
            NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_FLAGS,
                Noggit::ActionModalityControllers::eCTRL
                | Noggit::ActionModalityControllers::eLMB);
            mapView()->getWorld()->mapIndex.setFlag(false, mapView()->cursorPosition(), 0x2);
        }
    }
}
