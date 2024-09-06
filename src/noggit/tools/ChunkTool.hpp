// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Tool.hpp>

namespace Noggit
{
    namespace Ui::Tools::ChunkManipulator
    {
        class ChunkManipulatorPanel;
    }

    class ChunkTool final : public Tool
    {
    public:
        ChunkTool(MapView* mapView);
        ~ChunkTool();

        [[nodiscard]]
        virtual char const* name() const override;

        [[nodiscard]]
        virtual editing_mode editingMode() const override;

        [[nodiscard]]
        virtual Ui::FontNoggit::Icons icon() const override;

        void setupUi(Ui::Tools::ToolPanel* toolPanel) override;

    private:
        Ui::Tools::ChunkManipulator::ChunkManipulatorPanel* _chunkManipulator = nullptr;
    };
}