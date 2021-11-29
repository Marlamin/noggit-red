// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicBreakNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

LogicBreakNode::LogicBreakNode()
: LogicNodeBase()
{
  setName("Logic :: Break");
  setCaption("Logic :: Break");
  _validation_state = NodeValidationState::Valid;
  setInterpreterToken(NodeInterpreterTokens::BREAK);

  addPort<LogicData>(PortType::In, "Logic", true);
}

void LogicBreakNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  setDoBreak(logic->value());
}

NodeValidationState LogicBreakNode::validate()
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
