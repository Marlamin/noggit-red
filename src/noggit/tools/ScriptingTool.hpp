// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

namespace Noggit
{
    namespace Scripting
    {
        class scripting_tool;
    }

    class ScriptingTool final : public Tool
    {
    public:
        ScriptingTool(MapView* mapView);
        ~ScriptingTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

        [[nodiscard]]
        ToolDrawParameters drawParameters() const override;

        void onTick(float deltaTime, TickParameters const& params) override;

    private:
        Scripting::scripting_tool* _scriptingTool = nullptr;
    };
}