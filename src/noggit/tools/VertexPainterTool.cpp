// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "VertexPainterTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/ShaderTool.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

#include <random>

namespace Noggit
{
    VertexPainterTool::VertexPainterTool(MapView* mapView)
        : Tool{ mapView }
    {
        addHotkey("addColor"_hash, Hotkey{
            .onPress = [=] { _shaderTool->addColorToPalette(); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::mccv && !NOGGIT_CUR_ACTION; },
            });
    }

    VertexPainterTool::~VertexPainterTool()
    {
        delete _shaderTool;
    }

    char const* VertexPainterTool::name() const
    {
        return "Vertex Painter";
    }

    editing_mode VertexPainterTool::editingMode() const
    {
        return editing_mode::mccv;
    }

    Ui::FontNoggit::Icons VertexPainterTool::icon() const
    {
        return Ui::FontNoggit::TOOL_VERTEX_PAINT;
    }

    void VertexPainterTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _shaderTool = new Noggit::Ui::ShaderTool(mapView(), mapView());
        toolPanel->registerTool(name(), _shaderTool);
    }

    ToolDrawParameters VertexPainterTool::drawParameters() const
    {
        return
        {
            .radius = _shaderTool->brushRadius(),
            .cursor_type = (_shaderTool->getImageMaskSelector()->isEnabled()) ? CursorType::STAMP : CursorType::CIRCLE,
            .cursor_color = _shaderTool->shaderColor(),
        };
    }

    void VertexPainterTool::onSelected()
    {
        if (_shaderTool->getImageMaskSelector()->isEnabled())
        {
            _shaderTool->updateMaskImage();
        }
    }

    void VertexPainterTool::randomizeShaderRotation()
    {
        auto image_mask_selector = _shaderTool->getImageMaskSelector();
        if (!image_mask_selector->getRandomizeRotation())
            return;

        unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
        std::mt19937 gen(ms);
        std::uniform_int_distribution<> uid(0, 360);

        image_mask_selector->setRotation(uid(gen));
    }

    void VertexPainterTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (params.underMap || !params.left_mouse)
        {
            return;
        }

        for (auto& selection : mapView()->getWorld()->current_selection())
        {
            if (selection.index() != eEntry_MapChunk)
            {
                continue;
            }

            if ((params.mod_shift_down ^ params.mod_ctrl_down) == 0)
            {
                continue;
            }

            auto mv = mapView();
            bool add = params.mod_shift_down && !params.mod_ctrl_down;
            auto flags = add ? Noggit::ActionModalityControllers::eSHIFT : Noggit::ActionModalityControllers::eCTRL;

            auto image_mask_selector = _shaderTool->getImageMaskSelector();

            if (NOGGIT_CUR_ACTION
                && image_mask_selector->isEnabled()
                && !image_mask_selector->getBrushMode())
            {
                break;
            }

            auto action = NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_VERTEX_COLOR,
                flags | Noggit::ActionModalityControllers::eLMB);

            action->setPostCallback([this] { randomizeShaderRotation(); });

            if (image_mask_selector->isEnabled() && !image_mask_selector->getBrushMode())
                action->setBlockCursor(true);

            _shaderTool->changeShader(mv->getWorld(), mv->cursorPosition(), deltaTime, add);
        }
    }

    void VertexPainterTool::onMousePress(MousePressParameters const& params)
    {
        if (params.button != Qt::MouseButton::MiddleButton)
        {
            return;
        }

        _shaderTool->pickColor(mapView()->getWorld(), mapView()->cursorPosition());
    }

    void VertexPainterTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.right_mouse)
        {
            if (params.mod_space_down && _shaderTool->getImageMaskSelector()->isEnabled())
            {
                auto action = NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                    Noggit::ActionModalityControllers::eRMB
                    | Noggit::ActionModalityControllers::eSPACE);
                _shaderTool->getImageMaskSelector()->setRotation(-params.relative_movement.dx() / XSENS * 10.f);
                action->setBlockCursor(true);
            }
        }

        if (params.left_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _shaderTool->changeRadius(params.relative_movement.dx() / XSENS);
            }

            if (params.mod_space_down)
            {
                _shaderTool->changeSpeed(params.relative_movement.dx() / XSENS);
            }
        }
    }
}