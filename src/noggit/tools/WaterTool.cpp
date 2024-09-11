// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "WaterTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/Water.h>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

namespace Noggit
{
    WaterTool::WaterTool(MapView* mapView)
        : Tool{ mapView }
    {
        addHotkey("toggleAngled"_hash, Hotkey{
            .onPress = [=] { _guiWater->toggle_angled_mode(); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::water && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("toggleLock"_hash, Hotkey{
            .onPress = [=] { _guiWater->toggle_lock(); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::water && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("lockCursor"_hash, Hotkey{
            .onPress = [=] { _guiWater->lockPos(mapView->cursorPosition()); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::water && !NOGGIT_CUR_ACTION; },
            });
    }

    WaterTool::~WaterTool()
    {
        delete _guiWater;
    }

    char const* WaterTool::name() const
    {
        return "Water Editor";
    }

    editing_mode WaterTool::editingMode() const
    {
        return editing_mode::water;
    }

    Ui::FontNoggit::Icons WaterTool::icon() const
    {
        return Ui::FontNoggit::TOOL_WATER_EDITOR;
    }

    void WaterTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _guiWater = new Noggit::Ui::water(&_displayedWaterLayer, &_displayAllWaterLayers, mapView());
        toolPanel->registerTool(this, _guiWater);

        auto mv = mapView();

        QObject::connect(_guiWater, &Noggit::Ui::water::regenerate_water_opacity
            , [=](float factor)
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_WATER);
                mv->getWorld()->autoGenWaterTrans(mv->getCamera()->position, factor);
                NOGGIT_ACTION_MGR->endAction();
            }
        );

        QObject::connect(_guiWater, &Noggit::Ui::water::crop_water
            , [=]
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_WATER);
                mv->getWorld()->CropWaterADT(mv->getCamera()->position);
                NOGGIT_ACTION_MGR->endAction();
            }
        );
    }

    ToolDrawParameters WaterTool::drawParameters() const
    {
        return
        {
            .radius = _guiWater->brushRadius(),
            .angle = _guiWater->angle(),
            .orientation = _guiWater->orientation(),
            .ref_pos = _guiWater->ref_pos(),
            .angled_mode = _guiWater->angled_mode(),
            .use_ref_pos = _guiWater->use_ref_pos(),
            .displayed_water_layer = _displayAllWaterLayers.get() ? -1 : static_cast<int>(_displayedWaterLayer.get()),
        };
    }

    void WaterTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (params.underMap || !params.left_mouse)
        {
            return;
        }

        auto mv = mapView();
        if (params.displayMode == display_mode::in_3D && !params.underMap)
        {
            if (params.mod_shift_down)
            {
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_WATER,
                    Noggit::ActionModalityControllers::eSHIFT
                    | Noggit::ActionModalityControllers::eLMB);
                _guiWater->paintLiquid(mv->getWorld(), mv->cursorPosition(), true);
            }
            else if (params.mod_ctrl_down)
            {
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_WATER,
                    Noggit::ActionModalityControllers::eCTRL
                    | Noggit::ActionModalityControllers::eLMB);
                _guiWater->paintLiquid(mv->getWorld(), mv->cursorPosition(), false);
            }
        }

        // HINT: this was originally at the very end of MapView::Tick. Do we need a new postTick method?
        // While testing I can't find any issues with the tools.
        // Using the camera's position from last frame doesn't seem to matter but it's technically incorrect for one frame!
        _guiWater->updatePos(mv->getCamera()->position);
    }

    void WaterTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.left_mouse && params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
        {
            _guiWater->changeRadius(params.relative_movement.dx() / XSENS);
        }
    }

    void WaterTool::onMouseWheel(MouseWheelParameters const& params)
    {
        auto&& delta_for_range
        ([&](float range)
            {
                //! \note / 8.f for degrees, / 40.f for smoothness
                return (params.mod_ctrl_down ? 0.01f : 0.1f)
                    * range
                    // alt = horizontal delta
                    * (params.mod_alt_down ? params.event.angleDelta().x() : params.event.angleDelta().y())
                    / 320.f
                    ;
            }
        );

        if (params.mod_alt_down)
        {
            _guiWater->changeOrientation(delta_for_range(360.f));
        }
        else if (params.mod_shift_down)
        {
            _guiWater->changeAngle(delta_for_range(89.f));
        }
        else if (params.mod_space_down)
        {
            //! \note not actual range
            _guiWater->change_height(delta_for_range(40.f));
        }
    }
}
