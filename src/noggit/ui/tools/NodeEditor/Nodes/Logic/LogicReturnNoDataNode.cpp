// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicReturnNoDataNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

LogicReturnNoDataNode::LogicReturnNoDataNode()
: LogicNodeBase()
{
  setName("Logic :: ReturnNoData");
  setCaption("Logic :: ReturnNoData");
  _validation_state = NodeValidationState::Valid;
  setInterpreterToken(NodeInterpreterTokens::RETURN_NO_DATA);

  addPort<LogicData>(PortType::In, "Logic", true);
}

void LogicReturnNoDataNode::compute()
{

}

NodeValidationState LogicReturnNoDataNode::validate()
{
  setValidationState(NodeValidationState::Valid);
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input");
  }

  return _validation_state;
}
