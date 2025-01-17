// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ContextNodeBase.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>

namespace Noggit
{
  namespace Ui::Tools::NodeEditor::Nodes
  {
    ContextNodeBase::ContextNodeBase()
    {
    }

    NodeValidationState ContextNodeBase::validate()
    {
      if (!gCurrentContext->getWorld() || !gCurrentContext->getViewport())
      {
        setValidationState(NodeValidationState::Error);
        setValidationMessage("Error: this node requires a world context to run.");
      }

      return _validation_state;
    }
  }
}
