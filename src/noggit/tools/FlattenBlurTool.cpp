// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "FlattenBlurTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/Input.hpp>
#include <noggit/MapView.h>
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/ui/tools/ViewToolbar/Ui/ViewToolbar.hpp>

namespace Noggit
{
    FlattenBlurTool::FlattenBlurTool(MapView* mapView)
        : Tool{ mapView }
    {
        addHotkey("nextType"_hash, Hotkey{
            .onPress = [this] { _flattenTool->nextFlattenType(); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("toggleAngle"_hash, Hotkey{
            .onPress = [this] { _flattenTool->toggleFlattenAngle(); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("nextMode"_hash, Hotkey{
            .onPress = [this, mv = mapView]
            {
                mv->getLeftSecondaryViewToolbar()->nextFlattenMode();
                _flattenTool->nextFlattenMode();
            },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("toggleLock"_hash, Hotkey{
            .onPress = [this] { _flattenTool->toggleFlattenLock(); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("lockCursor"_hash, Hotkey{
            .onPress = [this, mv = mapView] { _flattenTool->lockPos(mv->cursorPosition()); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("increaseRadius"_hash, Hotkey{
            .onPress = [this] { _flattenTool->changeRadius(0.01f); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("decreaseRadius"_hash, Hotkey{
            .onPress = [this] { _flattenTool->changeRadius(-0.01f); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::flatten_blur && !NOGGIT_CUR_ACTION; },
            });
    }

    FlattenBlurTool::~FlattenBlurTool()
    {
        delete _flattenTool;
    }

    char const* FlattenBlurTool::name() const
    {
        return "Flatten | Blur";
    }

    editing_mode FlattenBlurTool::editingMode() const
    {
        return editing_mode::flatten_blur;
    }

    Ui::FontNoggit::Icons FlattenBlurTool::icon() const
    {
        return Ui::FontNoggit::TOOL_FLATTEN_BLUR;
    }

    void FlattenBlurTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _flattenTool = new Noggit::Ui::flatten_blur_tool(mapView());
        toolPanel->registerTool(name(), _flattenTool);
    }

    void FlattenBlurTool::postUiSetup()
    {
        QObject::connect(mapView()->getLeftSecondaryViewToolbar()
            , &Ui::Tools::ViewToolbar::Ui::ViewToolbar::updateStateRaise
            , [this](bool newState)
            {
                _flattenTool->_flatten_mode.raise = newState;
            }
        );

        QObject::connect(mapView()->getLeftSecondaryViewToolbar()
            , &Ui::Tools::ViewToolbar::Ui::ViewToolbar::updateStateLower
            , [this](bool newState)
            {
                _flattenTool->_flatten_mode.lower = newState;
            }
        );
    }

    void FlattenBlurTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (!mapView()->getWorld()->has_selection() || !params.left_mouse)
        {
            return;
        }

        if (params.displayMode == display_mode::in_3D && !params.underMap)
        {
            if (params.mod_shift_down)
            {
                NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN,
                    Noggit::ActionModalityControllers::eSHIFT
                    | Noggit::ActionModalityControllers::eLMB);
                _flattenTool->flatten(mapView()->getWorld(), mapView()->cursorPosition(), deltaTime);
            }
            else if (params.mod_ctrl_down)
            {

                NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN,
                    Noggit::ActionModalityControllers::eCTRL
                    | Noggit::ActionModalityControllers::eLMB);
                _flattenTool->blur(mapView()->getWorld(), mapView()->cursorPosition(), deltaTime);
            }
        }
    }

    ToolDrawParameters FlattenBlurTool::drawParameters() const
    {
        return
        {
            .radius = _flattenTool->brushRadius(),
            .angle = _flattenTool->angle(),
            .orientation = _flattenTool->orientation(),
            .ref_pos = _flattenTool->ref_pos(),
            .angled_mode = _flattenTool->angled_mode(),
            .use_ref_pos = _flattenTool->use_ref_pos(),
        };
    }

    void FlattenBlurTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.left_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _flattenTool->changeRadius(params.relative_movement.dx() / XSENS);
            }

            if (params.mod_space_down)
            {
                _flattenTool->changeSpeed(params.relative_movement.dx() / 30.0f);
            }
        }
    }

    void FlattenBlurTool::onMouseWheel(MouseWheelParameters const& params)
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
            _flattenTool->changeOrientation(delta_for_range(360.f));
        }
        else if (params.mod_shift_down)
        {
            _flattenTool->changeAngle(delta_for_range(89.f));
        }
        else if (params.mod_space_down)
        {
            //! \note not actual range
            _flattenTool->changeHeight(delta_for_range(40.f));
        }
    }
}
