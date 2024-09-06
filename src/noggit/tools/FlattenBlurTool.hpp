// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

namespace Noggit
{
    namespace Ui
    {
        class flatten_blur_tool;
    }

    class FlattenBlurTool final : public Tool
    {
    public:
        FlattenBlurTool(MapView* mapView);
        ~FlattenBlurTool();

        [[nodiscard]]
        char const* name() const override;

        [[nodiscard]]
        editing_mode editingMode() const override;

        [[nodiscard]]
        Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        void postUiSetup() override;

        void onTick(float deltaTime, TickParameters const& params) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void onMouseMove(MouseMoveParameters const& params) override;

        void onMouseWheel(MouseWheelParameters const& params) override;

    private:
        Ui::flatten_blur_tool* _flattenTool = nullptr;
    };
}