// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

namespace Noggit
{
    namespace Ui
    {
        class ShaderTool;
    }

    class VertexPainterTool final : public Tool
    {
    public:
        VertexPainterTool(MapView* mapView);
        ~VertexPainterTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        virtual void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void onSelected() override;

        void onTick(float deltaTime, TickParameters const& params) override;

        void onMousePress(MousePressParameters const& params) override;

        void onMouseMove(MouseMoveParameters const& params) override;

    private:
        Noggit::Ui::ShaderTool* _shaderTool = nullptr;

        void randomizeShaderRotation();
    };
}