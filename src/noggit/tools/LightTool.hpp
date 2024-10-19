// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

namespace Noggit
{
    namespace Ui::Tools
    {
        class LightEditor;
    }

    class LightTool final : public Tool
    {
    public:
        LightTool(MapView* mapView);
        ~LightTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        void onTick(float deltaTime, TickParameters const& params) override;

        void onSelected() override;

        void onDeselected() override;

    private:
        Ui::Tools::LightEditor* _lightEditor = nullptr;
    };
}