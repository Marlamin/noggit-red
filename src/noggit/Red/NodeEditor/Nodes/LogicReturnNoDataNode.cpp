// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicReturnNoDataNode.hpp"

#include "BaseNode.inl"
#include "noggit/Red/NodeEditor/Nodes/Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

LogicReturnNoDataNode::LogicReturnNoDataNode()
: LogicNodeBase()
{
  setName("LogicReturnNoDataNode");
  setCaption("Return");
  _validation_state = NodeValidationState::Valid;

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




