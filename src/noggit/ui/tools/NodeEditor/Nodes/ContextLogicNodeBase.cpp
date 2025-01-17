// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ContextLogicNodeBase.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>

namespace Noggit
{
  namespace Ui::Tools::NodeEditor::Nodes
  {
    Noggit::Ui::Tools::NodeEditor::Nodes::ContextLogicNodeBase::ContextLogicNodeBase()
    {
    }

    NodeValidationState ContextLogicNodeBase::validate()
    {
      if (!gCurrentContext->getWorld() || !gCurrentContext->getViewport())
      {
        setValidationState(NodeValidationState::Error);
        setValidationMessage("Error: this node requires a world context to run.");
        return _validation_state;
      }

      return LogicNodeBase::validate();
    }

    QJsonObject ContextLogicNodeBase::save() const
    {
      QJsonObject json_obj = BaseNode::save();

      for (int i = 0; i < _in_ports.size(); ++i)
      {
        defaultWidgetToJson(PortType::In, i, json_obj, _in_ports[i].caption);
      }

      return json_obj;
    }

    void ContextLogicNodeBase::restore(QJsonObject const& json_obj)
    {
      BaseNode::restore(json_obj);

      for (int i = 0; i < _in_ports.size(); ++i)
      {
        defaultWidgetFromJson(PortType::In, i, json_obj, _in_ports[i].caption);
      }

    }

  }
}
