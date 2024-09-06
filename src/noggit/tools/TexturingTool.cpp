// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "FlattenBlurTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/Input.hpp>
#include <noggit/MapView.h>
#include <noggit/Selection.h>
#include <noggit/ui/texturing_tool.hpp>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/ui/TexturePicker.h>
#include <noggit/ui/texture_palette_small.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>
#include <noggit/ui/tools/AssetBrowser/Ui/AssetBrowser.hpp>
#include <noggit/ui/windows/noggitWindow/NoggitWindow.hpp>
#include "TexturingTool.hpp"
#include <noggit/ui/tools/ViewToolbar/Ui/ViewToolbar.hpp>

#include <QDockWidget>
#include <QMenu>

#include <random>

namespace Noggit
{
    TexturingTool::TexturingTool(MapView* mapView)
        : Tool{ mapView }
    {
        addHotkey("toggleTool"_hash, Hotkey{
            .onPress = [=] { _texturingTool->toggle_tool(); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("setBrushLevelMinMax"_hash, Hotkey{
            .onPress = [=] { _texturingTool->toggle_brush_level_min_max(); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("increaseRadius"_hash, Hotkey{
            .onPress = [=] { _texturingTool->change_radius(0.1f); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("decreaseRadius"_hash, Hotkey{
            .onPress = [=] { _texturingTool->change_radius(-0.1f); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("setBrushLevel0Pct"_hash, Hotkey{
            .onPress = [=] { _texturingTool->set_brush_level(0.0f); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("setBrushLevel25Pct"_hash, Hotkey{
            .onPress = [=] { _texturingTool->set_brush_level(255.0f * 0.25f); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("setBrushLevel50Pct"_hash, Hotkey{
            .onPress = [=] { _texturingTool->set_brush_level(255.0f * 0.5f); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("setBrushLevel75Pct"_hash, Hotkey{
            .onPress = [=] { _texturingTool->set_brush_level(255.0f * 0.75f); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("setBrushLevel100Pct"_hash, Hotkey{
            .onPress = [=] { _texturingTool->set_brush_level(255.0f); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        addHotkey("toggleTexturePalette"_hash, Hotkey{
            .onPress = [=] { _show_texture_palette_small_window.toggle(); },
            .condition = [=] { return mapView->get_editing_mode() == editing_mode::paint && !NOGGIT_CUR_ACTION; },
            });

        QObject::connect(mapView
            , &MapView::selectionUpdated
            , [=](std::vector<selection_type>& selection)
            {
                if (_texturePickerNeedUpdate)
                {
                    _texturePickerDock->setVisible(true);
                    _texturePicker->setMainTexture(_texturingTool->_current_texture);
                    _texturePicker->getTextures(*selection.begin());

                    _texturePickerNeedUpdate = false;
                }
            }
        );
    }

    TexturingTool::~TexturingTool()
    {
        delete _texturePickerDock;
        delete _texturePaletteDock;
        delete _textureBrowserDock;
        delete _texturingTool;
    }

    char const* TexturingTool::name() const
    {
        return "Texture Painter";
    }

    editing_mode TexturingTool::editingMode() const
    {
        return editing_mode::paint;
    }

    Ui::FontNoggit::Icons TexturingTool::icon() const
    {
        return Ui::FontNoggit::TOOL_TEXTURE_PAINT;
    }

    void TexturingTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        auto mv = mapView();
        /* Tool */
        _texturingTool = new Ui::texturing_tool(&mv->getCamera()->position, mv, &_show_texture_palette_small_window, mv);
        toolPanel->registerTool(name(), _texturingTool);

        // Connects
        QObject::connect(_texturingTool->texture_swap_tool()->texture_display()
            , &Noggit::Ui::current_texture::texture_dropped
            , [=](std::string const& filename)
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());

                _texturingTool->texture_swap_tool()->set_texture(filename);
            }
        );

        QObject::connect(_texturingTool->_current_texture
            , &Noggit::Ui::current_texture::texture_dropped
            , [=](std::string const& filename)
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());

                Noggit::Ui::selected_texture::set({ filename, mv->getRenderContext() });
            }
        );

        QObject::connect(_texturingTool->_current_texture, &Noggit::Ui::current_texture::clicked
            , [=]
            {
                _textureBrowserDock->setVisible(!_textureBrowserDock->isVisible());
            }
        );

        QObject::connect(_texturingTool, &Ui::texturing_tool::texturePaletteToggled,
            [=]()
            {
                _show_texture_palette_small_window.set(!_show_texture_palette_small_window.get());
            });

        /* Additional tools */

        /* Texture Browser */

        // Dock
        _textureBrowserDock = new QDockWidget("Texture Browser", mv);
        _textureBrowserDock->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable
            | QDockWidget::DockWidgetClosable);
        _textureBrowserDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea);
        mv->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, _textureBrowserDock);
        _textureBrowserDock->hide();

        QObject::connect(_textureBrowserDock, &QDockWidget::visibilityChanged,
            [=](bool visible)
            {
                if (mv->isUiHidden())
                    return;

                mv->settings()->setValue("map_view/texture_browser", visible);
                mv->settings()->sync();
            });

        QObject::connect(mv, &QObject::destroyed, _textureBrowserDock, &QObject::deleteLater);
        // End Dock

        _texturePalette = new Noggit::Ui::tileset_chooser(mv);
        _textureBrowserDock->setWidget(_texturePalette);
        QObject::connect(mv, &QObject::destroyed, _texturePalette, &QObject::deleteLater);

        QObject::connect(_texturePalette, &Noggit::Ui::tileset_chooser::selected
            , [=](std::string const& filename)
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());

                Noggit::Ui::selected_texture::set({ filename, mv->getRenderContext() });
                _texturingTool->_current_texture->set_texture(filename);
                _texturePicker->setMainTexture(_texturingTool->_current_texture);
                _texturePicker->updateSelection();
            }
        );

        QObject::connect(_texturePalette, &Noggit::Ui::widget::visibilityChanged
            , &_show_texture_palette_window, &Noggit::BoolToggleProperty::set
        );

        QObject::connect(&_show_texture_palette_window, &Noggit::BoolToggleProperty::changed
            , [this, mv]
            {
                if ((mv->get_editing_mode() == editing_mode::paint || mv->get_editing_mode() == editing_mode::stamp)
                    && !mv->isUiHidden())
                {
                    _textureBrowserDock->setVisible(_show_texture_palette_window.get());
                }
                else
                {
                    QSignalBlocker const _(_show_texture_palette_window);
                    _show_texture_palette_window.set(false);
                }
            }
        );

        /* Texture Palette Small */
        _texturePaletteSmall = new Noggit::Ui::texture_palette_small(mv->project(), mv->getWorld()->getMapID(), mv);

        // Dock
        _texturePaletteDock = new QDockWidget("Texture Palette", mv);
        _texturePaletteDock->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable
            | QDockWidget::DockWidgetClosable
        );

        _texturePaletteDock->setWidget(_texturePaletteSmall);
        _texturePaletteDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
        _texturePaletteDock->hide();

        QObject::connect(mv, &QObject::destroyed, _texturePaletteDock, &QObject::deleteLater);

        mv->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, _texturePaletteDock);
        // End Dock

        QObject::connect(_texturePaletteDock, &QDockWidget::visibilityChanged,
            [=](bool visible)
            {
                if (mv->isUiHidden())
                    return;

                mv->settings()->setValue("map_view/texture_palette", visible);
                mv->settings()->sync();
            });

        QObject::connect(_texturePaletteSmall, &Noggit::Ui::texture_palette_small::selected
            , [=](std::string const& filename)
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());

                Noggit::Ui::selected_texture::set({ filename, mv->getRenderContext() });
                _texturingTool->_current_texture->set_texture(filename);
            }
        );
        QObject::connect(mv, &QObject::destroyed, _texturePaletteSmall, &QObject::deleteLater);

        QObject::connect(&_show_texture_palette_small_window, &Noggit::BoolToggleProperty::changed
            , _texturePaletteDock, [=]
            {
                QSignalBlocker const blocker(_show_texture_palette_small_window);
                if (mv->get_editing_mode() == editing_mode::paint && !mv->isUiHidden())
                {
                    _texturePaletteDock->setVisible(_show_texture_palette_small_window.get());
                }
                else
                {
                    _show_texture_palette_small_window.set(false);
                }
            }
        );
        QObject::connect(_texturePaletteDock, &QDockWidget::visibilityChanged
            , &_show_texture_palette_small_window, &Noggit::BoolToggleProperty::set
        );

        QObject::connect(_texturingTool->_current_texture, &Noggit::Ui::current_texture::texture_updated
            , [=]()
            {
                mv->getWorld()->notifyTileRendererOnSelectedTextureChange();
                _texturingTool->getGroundEffectsTool()->TextureChanged();
            }
        );

        /* Texture Picker */

        // Dock
        _texturePickerDock = new QDockWidget("Texture picker", mv);
        _texturePickerDock->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable
            | QDockWidget::DockWidgetClosable);
        mv->mainWindow()->addDockWidget(Qt::BottomDockWidgetArea, _texturePickerDock);
        _texturePickerDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
        _texturePickerDock->setFloating(true);
        _texturePickerDock->hide();
        QObject::connect(mv, &QObject::destroyed, _texturePickerDock, &QObject::deleteLater);
        // End Dock

        _texturePicker = new Noggit::Ui::texture_picker(_texturingTool->_current_texture, mv);
        _texturePickerDock->setWidget(_texturePicker);
        QObject::connect(mv, &QObject::destroyed, _texturePicker, &QObject::deleteLater);

        QObject::connect(_texturePicker
            , &Noggit::Ui::texture_picker::set_texture
            , [=](scoped_blp_texture_reference texture)
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());
                Noggit::Ui::selected_texture::set(std::move(texture));
            }
        );
        QObject::connect(_texturePicker, &Noggit::Ui::texture_picker::shift_left
            , [=]
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());
                _texturePicker->shiftSelectedTextureLeft();
            }
        );
        QObject::connect(_texturePicker, &Noggit::Ui::texture_picker::shift_right
            , [=]
            {
                mv->makeCurrent();
                OpenGL::context::scoped_setter const _(::gl, mv->context());
                _texturePicker->shiftSelectedTextureRight();
            }
        );
    }

    void TexturingTool::registerMenuItems(QMenu* menu)
    {
        addMenuTitle(menu, "Texture Painter");
        addMenuItem(menu, "Texture Browser", Qt::Key_X, _show_texture_palette_window);
        addMenuItem(menu, "Texture palette", _show_texture_palette_small_window);
    }

    ToolDrawParameters TexturingTool::drawParameters() const
    {
        auto cursorType = CursorType::CIRCLE;
        if (_texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint && _texturingTool->getImageMaskSelector()->isEnabled())
            cursorType = CursorType::STAMP;

        return
        {
            .radius = _texturingTool->brush_radius(),
            .inner_radius = _texturingTool->hardness(),
            .show_unpaintable_chunks = _texturingTool->show_unpaintable_chunks(),
            .cursor_type = cursorType,
        };
    }

    void TexturingTool::onSelected()
    {
        auto mv = mapView();
        if (_texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint && _texturingTool->getImageMaskSelector()->isEnabled())
        {
            _texturingTool->updateMaskImage();
        }
        else if (_texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::ground_effect)
        {
            _texturingTool->getGroundEffectsTool()->updateTerrainUniformParams();
        }

        bool use_classic_ui = mv->settings()->value("classicUI", false).toBool();
        if (use_classic_ui)
        {
            if (_texturingTool->show_unpaintable_chunks())
            {
                mv->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = true;
            }
        }
        else
        {
            if (mv->getLeftSecondaryViewToolbar()->showUnpaintableChunk())
            {
                mv->getWorld()->renderer()->getTerrainParamsUniformBlock()->draw_paintability_overlay = true;
            }
        }

        _textureBrowserDock->setVisible(!mv->isUiHidden() && mv->settings()->value("map_view/texture_browser", false).toBool());
        _texturePaletteDock->setVisible(!mv->isUiHidden() && mv->settings()->value("map_view/texture_palette", false).toBool());
    }

    void TexturingTool::onDeselected()
    {
        _texturingTool->getGroundEffectsTool()->hide();
        _textureBrowserDock->setVisible(false);
        _texturePaletteDock->setVisible(false);
    }

    void TexturingTool::onTick(float deltaTime, TickParameters const& params)
    {
        if (!params.left_mouse)
        {
            return;
        }

        auto mv = mapView();

        if (_texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::ground_effect)
        {
            if (params.mod_shift_down)
            {
                if (_texturingTool->getGroundEffectsTool()->brush_mode() == Noggit::Ui::ground_effect_brush_mode::exclusion)
                {
                    NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNK_DOODADS_EXCLUSION,
                        Noggit::ActionModalityControllers::eSHIFT
                        | Noggit::ActionModalityControllers::eLMB);
                    mv->getWorld()->paintGroundEffectExclusion(mv->cursorPosition(), _texturingTool->getGroundEffectsTool()->radius(), true);
                    // mv->getWorld()->setHole(mv->cursorPosition(), holeTool->brushRadius(), _mod_alt_down, false);
                }
                else if (_texturingTool->getGroundEffectsTool()->brush_mode() == Noggit::Ui::ground_effect_brush_mode::effect)
                {

                }

            }
            else if (params.mod_ctrl_down && !params.underMap)
            {
                NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNK_DOODADS_EXCLUSION,
                    Noggit::ActionModalityControllers::eCTRL
                    | Noggit::ActionModalityControllers::eLMB);
                mv->getWorld()->paintGroundEffectExclusion(mv->cursorPosition(), _texturingTool->getGroundEffectsTool()->radius(), false);
            }
        }
        else
        {
            if (params.mod_shift_down && params.mod_ctrl_down && params.mod_alt_down)
            {
                // clear chunk texture
                if (!params.underMap)
                {
                    NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_TEXTURE,
                        Noggit::ActionModalityControllers::eSHIFT
                        | Noggit::ActionModalityControllers::eCTRL
                        | Noggit::ActionModalityControllers::eALT
                        | Noggit::ActionModalityControllers::eLMB);

                    mv->getWorld()->eraseTextures(mv->cursorPosition());
                }
            }
            else if (params.mod_ctrl_down && !mv->isUiHidden())
            {
                _texturePickerNeedUpdate = true;
                // Pick texture
                // _texturePickerDock->setVisible(true);
                // _texturePicker->setMainTexture(_texturingTool->_current_texture);
                // _texturePicker->getTextures(selection);
            }
            else  if (params.mod_shift_down && !!Noggit::Ui::selected_texture::get())
            {
                if ((params.displayMode == display_mode::in_3D && !params.underMap) || params.displayMode == display_mode::in_2D)
                {
                    auto image_mask_selector = _texturingTool->getImageMaskSelector();

                    if (NOGGIT_CUR_ACTION
                        && _texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint
                        && image_mask_selector->isEnabled()
                        && !image_mask_selector->getBrushMode())
                    {
                        return;
                    }

                    auto action = NOGGIT_ACTION_MGR->beginAction(mv, Noggit::ActionFlags::eCHUNKS_TEXTURE,
                        Noggit::ActionModalityControllers::eSHIFT
                        | Noggit::ActionModalityControllers::eLMB);

                    action->setPostCallback([this] { randomizeTexturingRotation(); });

                    if (_texturingTool->getTexturingMode() == Noggit::Ui::texturing_mode::paint
                        && image_mask_selector->isEnabled()
                        && !image_mask_selector->getBrushMode())
                        action->setBlockCursor(true);

                    _texturingTool->paint(mv->getWorld(), mv->cursorPosition(), deltaTime, *Noggit::Ui::selected_texture::get());
                }
            }
        }
    }

    void TexturingTool::onMousePress(MousePressParameters const& params)
    {
        if (params.button != Qt::MouseButton::LeftButton || !params.mod_ctrl_down)
        {
            return;
        }

        mapView()->doSelection(false, false);
    }

    void TexturingTool::onMouseMove(MouseMoveParameters const& params)
    {
        if (params.right_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _texturingTool->change_hardness(params.relative_movement.dx() / 300.0f);
            }

            if (params.mod_space_down)
            {
                if (_texturingTool->getImageMaskSelector()->isEnabled())
                {
                    auto action = NOGGIT_ACTION_MGR->beginAction(mapView(), Noggit::ActionFlags::eDO_NOT_WRITE_HISTORY,
                        Noggit::ActionModalityControllers::eRMB
                        | Noggit::ActionModalityControllers::eSPACE);
                    _texturingTool->getImageMaskSelector()->setRotation(-params.relative_movement.dx() / XSENS * 10.f);
                    action->setBlockCursor(true);
                }
            }
        }

        if (params.left_mouse)
        {
            if (params.mod_alt_down && !params.mod_shift_down && !params.mod_ctrl_down)
            {
                _texturingTool->change_radius(params.relative_movement.dx() / XSENS);
            }

            if (params.mod_space_down)
            {
                _texturingTool->change_pressure(params.relative_movement.dx() / 300.0f);
            }
        }
    }

    void TexturingTool::onMouseWheel(MouseWheelParameters const& params)
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

        if (params.mod_space_down)
        {
            _texturingTool->change_brush_level(delta_for_range(255.f));
        }
        else if (params.mod_alt_down)
        {
            _texturingTool->change_spray_size(delta_for_range(39.f));
        }
        else if (params.mod_shift_down)
        {
            _texturingTool->change_spray_pressure(delta_for_range(10.f));
        }
    }

    void TexturingTool::hidePopups()
    {
        _texturePaletteSmall->hide();
        _texturePickerDock->hide();
        _textureBrowserDock->hide();
    }

    void TexturingTool::randomizeTexturingRotation()
    {
        auto image_mask_selector = _texturingTool->getImageMaskSelector();
        if (!image_mask_selector->getRandomizeRotation())
            return;

        unsigned int ms = static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch());
        std::mt19937 gen(ms);
        std::uniform_int_distribution<> uid(0, 360);

        image_mask_selector->setRotation(uid(gen));
    }
}