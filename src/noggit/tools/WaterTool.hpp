// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>
#include <noggit/unsigned_int_property.hpp>
#include <noggit/BoolToggleProperty.hpp>

namespace Noggit
{
    namespace Ui
    {
        class water;
    }

    class WaterTool final : public Tool
    {
    public:
        WaterTool(MapView* mapView);
        ~WaterTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        virtual void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void onTick(float deltaTime, TickParameters const& params) override;

        void onMouseMove(MouseMoveParameters const& params) override;

        void onMouseWheel(MouseWheelParameters const& params) override;

    private:
        Noggit::Ui::water* _guiWater = nullptr;
        Noggit::unsigned_int_property _displayedWaterLayer = { 0 };
        Noggit::BoolToggleProperty _displayAllWaterLayers = { true };
    };
}