// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "AreaTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/ZoneIDBrowser.h>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/World.h>

namespace Noggit
{
    AreaTool::AreaTool(MapView* mapView)
        : Tool{ mapView }
    {
        addHotkey("setAreaId"_hash, {
            .onPress = [=] {
              if (_selectedAreaId != -1)
              {
                NOGGIT_ACTION_MGR->beginAction(mapView, Noggit::ActionFlags::eCHUNKS_AREAID);
                mapView->getWorld()->setAreaID(mapView->getCamera()->position, _selectedAreaId, true);
                NOGGIT_ACTION_MGR->endAction();
              }},
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::areaid && !NOGGIT_CUR_ACTION; }
            });
    }

    AreaTool::~AreaTool()
    {
    }

    char const* AreaTool::name() const
    {
        return "Area Designator";
    }

    editing_mode AreaTool::editingMode() const
    {
        return editing_mode::areaid;
    }

    Ui::FontNoggit::Icons AreaTool::icon() const
    {
        return Ui::FontNoggit::TOOL_AREA_DESIGNATOR;
    }

    void AreaTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _areaTool = new Noggit::Ui::zone_id_browser(mapView());
        toolPanel->registerTool(this, _areaTool);

        _areaTool->setMapID(mapView()->getWorld()->getMapID());
        QObject::connect(_areaTool, &Noggit::Ui::zone_id_browser::selected
            , [this](int area_id) { _selectedAreaId = area_id; }
        );
    }

    ToolDrawParameters AreaTool::drawParameters() const
    {
        return ToolDrawParameters();
    }

    void AreaTool::registerMenuItems(QMenu* menu)
    {
        addMenuTitle(menu, "Area Designator");
        addMenuItem(menu, "Set Area ID", [=] {
            if (_selectedAreaId == -1)
            {
                return;
            }

            NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_AREAID);
            mapView()->getWorld()->setAreaID(mapView()->getCamera()->position, _selectedAreaId, true);
            NOGGIT_ACTION_MGR->endAction();
            });
    }

    void AreaTool::onSelected()
    {
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_areaid_overlay = true;
    }

    void AreaTool::onDeselected()
    {
        mapView()->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_areaid_overlay = false;
    }

    void AreaTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.left_mouse && params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
        {
            _areaTool->changeRadius(params.relative_movement.dx() / XSENS);
        }
    }

    void AreaTool::onMousePress(MousePressParameters const& params)
    {
      if (params.button != Qt::LeftButton)
      {
        return;
      }

      if (params.mod_shift_down)
      {
        NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_AREAID,
          Noggit::ActionModalityControllers::eSHIFT
          | Noggit::ActionModalityControllers::eLMB);
        // draw the selected AreaId on current selected chunk
        mapView()->getWorld()->setAreaID(mapView()->cursorPosition(), _selectedAreaId, false, _areaTool->brushRadius());
      }
      else if(params.mod_ctrl_down)
      {
        mapView()->doSelection(true);

        for (auto&& selection : mapView()->getWorld()->current_selection())
        {
          MapChunk* chnk(std::get<selected_chunk_type>(selection).chunk);
          int newID = chnk->getAreaID();
          _selectedAreaId = newID;
          _areaTool->setZoneID(newID);
        }
        return;
      }
    }
}
