// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RaiseLowerTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/Input.hpp>
#include <noggit/MapView.h>
#include <noggit/ui/TerrainTool.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

#include <random>

namespace Noggit
{
    RaiseLowerTool::RaiseLowerTool(MapView* mapView)
        : Tool{ mapView }
    {
        addHotkey("nextType"_hash, Hotkey{
            .onPress = [this] { _terrainTool->nextType(); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::ground && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("flatten"_hash, Hotkey{
            .onPress = [this] {
                NOGGIT_ACTION_MGR->beginAction(this->mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN);
                _terrainTool->flattenVertices(this->mapView()->getWorld());
                NOGGIT_ACTION_MGR->endAction();
            },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::ground && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("increaseRadius"_hash, Hotkey{
            .onPress = [this] { _terrainTool->changeRadius(0.01f); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::ground && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("decreaseRadius"_hash, Hotkey{
            .onPress = [this] { _terrainTool->changeRadius(-0.01f); },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::ground && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("clearVertexSelection"_hash, Hotkey{
            .onPress = [=] {
                NOGGIT_ACTION_MGR->beginAction(mapView, Noggit::ActionFlags::eVERTEX_SELECTION);
                mapView->getWorld()->clearVertexSelection();
                NOGGIT_ACTION_MGR->endAction();
            },
            .condition = [mapView] { return mapView->get_editing_mode() == editing_mode::ground && !NOGGIT_CUR_ACTION; },
            });
    }

    RaiseLowerTool::~RaiseLowerTool()
    {
        delete _terrainTool;
    }

    char const* RaiseLowerTool::name() const
    {
        return "Raise / Lower";
    }

    editing_mode RaiseLowerTool::editingMode() const
    {
        return editing_mode::ground;
    }

    Ui::FontNoggit::Icons RaiseLowerTool::icon() const
    {
        return Ui::FontNoggit::TOOL_RAISE_LOWER;
    }

    void RaiseLowerTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _terrainTool = new Noggit::Ui::TerrainTool{ mapView(), mapView() };
        toolPanel->registerTool(this, _terrainTool);

        QObject::connect(_terrainTool
            , &Noggit::Ui::TerrainTool::updateVertices
            , [mapView = mapView()](int vertex_mode, math::degrees const& angle, math::degrees const& orientation)
            {
                mapView->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mapView->context());

                mapView->getWorld()->orientVertices(vertex_mode == eVertexMode_Mouse
                    ? mapView->cursorPosition()
                    : mapView->getWorld()->vertexCenter()
                    , angle
                    , orientation
                );
            }
        );
    }

    ToolDrawParameters RaiseLowerTool::drawParameters() const
    {
        CursorType cursorType = CursorType::CIRCLE;
        if ((_terrainTool->_edit_type != eTerrainType_Vertex && _terrainTool->_edit_type != eTerrainType_Script) && _terrainTool->getImageMaskSelector()->isEnabled())
            cursorType = CursorType::STAMP;

        return ToolDrawParameters
        {
            .radius = _terrainTool->brushRadius(),
            .inner_radius = _terrainTool->innerRadius(),
            .cursor_type = cursorType,
            .terrain_type = _terrainTool->_edit_type,
        };
    }

    void RaiseLowerTool::onSelected()
    {
        if (_terrainTool->_edit_type != eTerrainType_Vertex
            || (_terrainTool->_edit_type != eTerrainType_Script && _terrainTool->getImageMaskSelector()->isEnabled()))
        {
            _terrainTool->updateMaskImage();
        }
    }

    void RaiseLowerTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (!params.left_mouse)
        {
            return;
        }

        if (params.displayMode == display_mode::in_3D && !params.underMap)
        {
            auto mask_selector = _terrainTool->getImageMaskSelector();
            auto mv = mapView();

            if (params.mod_shift_down && (!mask_selector->isEnabled() || mask_selector->getBrushMode()))
            {
                auto action = NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                    Noggit::ActionModalityControllers::eSHIFT
                    | Noggit::ActionModalityControllers::eLMB);

                action->setPostCallback([this] { randomizeTerrainRotation(); });

                _terrainTool->changeTerrain(mv->getWorld(), mv->cursorPosition(), 7.5f * deltaTime);
            }
            else if (params.mod_ctrl_down && (!mask_selector->isEnabled() || mask_selector->getBrushMode()))
            {
                auto action = NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_TERRAIN,
                    Noggit::ActionModalityControllers::eCTRL
                    | Noggit::ActionModalityControllers::eLMB);

                action->setPostCallback([this] { randomizeTerrainRotation(); });

                _terrainTool->changeTerrain(mv->getWorld(), mv->cursorPosition(), -7.5f * deltaTime);
            }
        }
    }

    void RaiseLowerTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.right_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                if (_terrainTool->_edit_type == eTerrainType_Vertex)
                {
                    NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN,
                        Noggit::ActionModalityControllers::eALT | Noggit::ActionModalityControllers::eRMB);
                    _terrainTool->changeOrientation(-params.relative_movement.dx() / XSENS * 4.5f);
                }
                else
                {
                    _terrainTool->changeInnerRadius(params.relative_movement.dx() / 100.0f);
                }
            }

            if (!params.mod_alt_down && params.mod_shift_down && !params.mod_ctrl_down)
            {
                if (_terrainTool->_edit_type == eTerrainType_Vertex)
                {
                    NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN,
                        Noggit::ActionModalityControllers::eSHIFT | Noggit::ActionModalityControllers::eRMB);
                    _terrainTool->moveVertices(mapView()->getWorld(), -params.relative_movement.dy() / YSENS);
                }
            }

            if (!params.mod_alt_down && !params.mod_shift_down && params.mod_ctrl_down)
            {
                if (_terrainTool->_edit_type == eTerrainType_Vertex)
                {
                    NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN,
                        Noggit::ActionModalityControllers::eCTRL |
                        Noggit::ActionModalityControllers::eRMB);
                    _terrainTool->changeAngle(-params.relative_movement.dy() / YSENS * 4.f);
                }
            }

            if (params.mod_space_down)
            {
                if (_terrainTool->_edit_type == eTerrainType_Vertex)
                {
                    NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN,
                        Noggit::ActionModalityControllers::eRMB
                        | Noggit::ActionModalityControllers::eSPACE);
                    _terrainTool->setOrientRelativeTo(mapView()->getWorld(), mapView()->cursorPosition());
                }
                else if (_terrainTool->getImageMaskSelector()->isEnabled())
                {
                    auto action = NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                        Noggit::ActionModalityControllers::eRMB
                        | Noggit::ActionModalityControllers::eSPACE);
                    _terrainTool->getImageMaskSelector()->setRotation(-params.relative_movement.dx() / XSENS * 10.f);
                    action->setBlockCursor(true);
                }
            }
        }

        if (params.left_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _terrainTool->changeRadius(params.relative_movement.dx() / XSENS);
            }

            if (params.mod_space_down)
            {
                _terrainTool->changeSpeed(params.relative_movement.dx() / 30.0f);
            }

            if (params.mod_shift_down && params.displayMode == display_mode::in_3D)
            {
                auto image_mask_selector = _terrainTool->getImageMaskSelector();
                if (_terrainTool->_edit_type != eTerrainType_Vertex && _terrainTool->_edit_type != eTerrainType_Script &&
                    image_mask_selector->isEnabled() && !image_mask_selector->getBrushMode())
                {
                    auto action = NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eCHUNKS_TERRAIN,
                        Noggit::ActionModalityControllers::eSHIFT
                        | Noggit::ActionModalityControllers::eLMB);

                    action->setPostCallback([this] { randomizeTerrainRotation(); });

                    _terrainTool->changeTerrain(mapView()->getWorld(), mapView()->cursorPosition(), params.relative_movement.dx() / 30.0f);
                }
            }
        }
    }

    void RaiseLowerTool::randomizeTerrainRotation()
    {
        auto image_mask_selector = _terrainTool->getImageMaskSelector();
        if (!image_mask_selector->getRandomizeRotation())
            return;

        unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
        std::mt19937 gen(ms);
        std::uniform_int_distribution<> uid(0, 360);

        image_mask_selector->setRotation(uid(gen));
    }
}
