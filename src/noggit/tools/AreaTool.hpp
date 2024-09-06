// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

namespace Noggit
{
    namespace Ui
    {
        class zone_id_browser;
    }

    class AreaTool final : public Tool
    {
    public:
        AreaTool(MapView* mapView);
        ~AreaTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void registerMenuItems(QMenu* menu) override;

        virtual void onSelected();

        virtual void onDeselected();

        void onTick(float deltaTime, TickParameters const& params) override;

        void onMouseMove(MouseMoveParameters const& params) override;

    private:
        Ui::zone_id_browser* _areaTool = nullptr;
        int _selectedAreaId = -1;
    };
}