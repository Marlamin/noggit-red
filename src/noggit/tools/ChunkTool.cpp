// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkTool.hpp"

#include <noggit/ActionManager.hpp>
#include <noggit/MapView.h>
#include <noggit/Input.hpp>
#include <noggit/ui/tools/ChunkManipulator/ChunkManipulatorPanel.hpp>
#include <noggit/ui/tools/ToolPanel/ToolPanel.hpp>

namespace Noggit
{
    ChunkTool::ChunkTool(MapView* mapView)
        : Tool{ mapView }
    {
    }

    ChunkTool::~ChunkTool()
    {
        delete _chunkManipulator;
    }

    char const* ChunkTool::name() const
    {
        return "Chunk Manipulator";
    }

    editing_mode ChunkTool::editingMode() const
    {
        return editing_mode::chunk;
    }

    Ui::FontNoggit::Icons ChunkTool::icon() const
    {
        return Ui::FontNoggit::INFO;
    }

    void ChunkTool::setupUi(Ui::Tools::ToolPanel* toolPanel)
    {
        _chunkManipulator = new Noggit::Ui::Tools::ChunkManipulator::ChunkManipulatorPanel(mapView(), mapView());
        toolPanel->registerTool(name(), _chunkManipulator);
    }
}