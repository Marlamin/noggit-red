// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

namespace Noggit
{
    namespace Ui
    {
        class TerrainTool;
    }

    class RaiseLowerTool final : public Tool
    {
    public:
        RaiseLowerTool(MapView* mapView);
        ~RaiseLowerTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void onSelected() override;

        void onTick(float deltaTime, TickParameters const& params) override;

        void onMouseMove(MouseMoveParameters const& params) override;

    private:
        Ui::TerrainTool* _terrainTool = nullptr;

        void randomizeTerrainRotation();
    };
}