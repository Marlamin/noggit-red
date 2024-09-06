// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>
#include <noggit/BoolToggleProperty.hpp>

class QDockWidget;

namespace Noggit
{
    namespace Ui
    {
        class texturing_tool;
        struct tileset_chooser;
        class texture_picker;
        class texture_palette_small;
    }

    class TexturingTool final : public Tool
    {
    public:
        TexturingTool(MapView* mapView);
        ~TexturingTool();

        [[nodiscard]]
        char const* name() const override;

        [[nodiscard]]
        editing_mode editingMode() const override;

        [[nodiscard]]
        Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        void registerMenuItems(QMenu* menu) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void onSelected() override;

        void onDeselected() override;

        void onTick(float deltaTime, TickParameters const& params) override;

        void onMousePress(MousePressParameters const& params) override;

        void onMouseMove(MouseMoveParameters const& params) override;

        void onMouseWheel(MouseWheelParameters const& params) override;

        void hidePopups() override;

    private:
        Ui::texturing_tool* _texturingTool = nullptr;
        QDockWidget* _textureBrowserDock = nullptr;
        Ui::tileset_chooser* _texturePalette = nullptr;
        Ui::texture_picker* _texturePicker = nullptr;
        Ui::texture_palette_small* _texturePaletteSmall = nullptr;
        QDockWidget* _texturePaletteDock = nullptr;
        QDockWidget* _texturePickerDock = nullptr;
        bool _texturePickerNeedUpdate = false;
        Noggit::BoolToggleProperty _show_texture_palette_window = { false };
        Noggit::BoolToggleProperty _show_texture_palette_small_window = { false };

        void randomizeTexturingRotation();
    };
}