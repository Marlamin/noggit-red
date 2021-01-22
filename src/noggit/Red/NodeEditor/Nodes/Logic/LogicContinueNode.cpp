// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicContinueNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

LogicContinueNode::LogicContinueNode()
: LogicNodeBase()
{
  setName("Logic :: Continue");
  setCaption("Logic :: Continue");
  _validation_state = NodeValidationState::Valid;
  setInterpreterToken(NodeInterpreterTokens::CONTINUE);

  addPort<LogicData>(PortType::In, "Logic", true);
}

void LogicContinueNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  setDoContinue(logic->value());
}

NodeValidationState LogicContinueNode::validate()
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
