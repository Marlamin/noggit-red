// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicBreakNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::PresetEditor::Nodes;

LogicBreakNode::LogicBreakNode()
: LogicNodeBase()
{
  setName("LogicBreakNode");
  setCaption("Break");
  _validation_state = NodeValidationState::Valid;

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

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _out_ports[1].out_value = std::make_shared<LogicData>(false);
    Q_EMIT dataUpdated(0);
    Q_EMIT dataUpdated(1);

    setLogicBranchToExecute(-1);
  }

  return _validation_state;
}


