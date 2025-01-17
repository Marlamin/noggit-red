// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "StampTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/Input.hpp>
#include <noggit/MapView.h>
#include <noggit/ui/tools/BrushStack/BrushStack.hpp>
#include <noggit/ui/tools/BrushStack/BrushStackItem.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/World.h>

#include <random>

namespace Noggit
{
    StampTool::StampTool(MapView* mapView)
        : Tool{ mapView }
    {
    }

    StampTool::~StampTool()
    {
        delete _stampTool;
    }

    char const* StampTool::name() const
    {
        return "Stamp Mode";
    }

    editing_mode StampTool::editingMode() const
    {
        return editing_mode::stamp;
    }

    Ui::FontNoggit::Icons StampTool::icon() const
    {
        return Ui::FontNoggit::TOOL_STAMP;
    }

    void StampTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _stampTool = new Noggit::Ui::Tools::BrushStack(mapView(), mapView());
        toolPanel->registerTool(this, _stampTool);

        QObject::connect(mapView(), &MapView::trySetBrushTexture, [=](QImage* brush, QWidget* sender) {
            auto mv = mapView();
            auto item = _stampTool->getActiveBrushItem();

            if (mv->get_editing_mode() != editing_mode::stamp || (item && item->getTool() == sender))
            {
                mv->setBrushTexture(brush);
            }
            });
    }

    ToolDrawParameters StampTool::drawParameters() const
    {
        return
        {
            .radius = _stampTool->getRadius(),
            .inner_radius = _stampTool->getInnerRadius(),
            .cursor_type = (_stampTool->getActiveBrushItem() && _stampTool->getActiveBrushItem()->isMaskEnabled()) ? CursorType::STAMP : CursorType::CIRCLE,
        };
    }

    void StampTool::onSelected()
    {
        if (_stampTool->getActiveBrushItem() && _stampTool->getActiveBrushItem()->isEnabled())
        {
            _stampTool->getActiveBrushItem()->updateMask();
        }
    }

    void StampTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (!mapView()->getWorld()->has_selection() || !params.left_mouse)
        {
            return;
        }

        auto mv = mapView();
        auto world = mv->getWorld();

        for (auto& selection : world->current_selection())
        {
            if (selection.index() != eEntry_MapChunk
                || params.displayMode != display_mode::in_3D
                || !(params.mod_shift_down || params.mod_ctrl_down || params.mod_alt_down)
                || !_stampTool->getBrushMode())
            {
                continue;
            }

            auto action = NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eNO_FLAG,
                Noggit::ActionModalityControllers::eSHIFT
                | Noggit::ActionModalityControllers::eLMB);

            if (!_stampTool->getBrushMode())
                action->setBlockCursor(true);

            _stampTool->execute(mv->cursorPosition(),
                world, deltaTime,
                params.mod_shift_down,
                params.mod_alt_down,
                params.mod_ctrl_down,
                world->isUnderMap(mv->cursorPosition()));
        }
    }

    void StampTool::onMouseMove(MouseMoveParameters const& params)
    {
        auto mv = mapView();
        if (params.right_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _stampTool->changeInnerRadius(params.relative_movement.dx() / 300.0f);
            }

            if (params.mod_space_down)
            {
                auto action = NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                    Noggit::ActionModalityControllers::eRMB
                    | Noggit::ActionModalityControllers::eSPACE);

                _stampTool->changeRotation(-params.relative_movement.dx() / XSENS * 10.f);
                action->setBlockCursor(true);
            }
        }

        if (params.left_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _stampTool->changeRadius(params.relative_movement.dx() / XSENS);
            }

            if (params.mod_space_down)
            {
                _stampTool->changeSpeed(params.relative_movement.dx() / XSENS);
            }

            if (params.mod_shift_down)
            {
                if(params.displayMode == display_mode::in_3D && !_stampTool->getBrushMode())
                {
                    auto action = NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eNO_FLAG,
                        Noggit::ActionModalityControllers::eSHIFT
                        | Noggit::ActionModalityControllers::eLMB);

                    action->setPostCallback([this] { randomizeStampRotation(); });
                    action->setBlockCursor(true);

                    _stampTool->execute(mv->cursorPosition()
                        , mv->getWorld()
                        , params.relative_movement.dx() / 30.0f
                        , params.mod_shift_down
                        , params.mod_alt_down
                        , params.mod_ctrl_down
                        , false);
                }
            }
        }
    }

    void StampTool::randomizeStampRotation()
    {
        if (!_stampTool->getRandomizeRotation())
            return;

        unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
        std::mt19937 gen(ms);
        std::uniform_int_distribution<> uid(0, 360);

        _stampTool->changeRotation(uid(gen));
    }
}
