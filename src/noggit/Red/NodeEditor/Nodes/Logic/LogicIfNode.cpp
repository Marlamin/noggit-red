#include "LogicIfNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

LogicIfNode::LogicIfNode()
: LogicNodeBase()
{
  setName("LogicIfNode");
  setCaption("Branch");
  setValidationState(NodeValidationState::Valid);

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<BooleanData>(PortType::In, "Boolean", true);
  addPort<LogicData>(PortType::Out, "True", true, ConnectionPolicy::One);
  addPort<LogicData>(PortType::Out, "False", true, ConnectionPolicy::One);

  setNLogicBranches(2);
}

void LogicIfNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if(!logic->value())
  {
    setLogicBranchToExecute(-1);
    return;
  }

  auto in_bool = static_cast<BooleanData*>(_in_ports[1].in_value.lock().get());

  if (!in_bool)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Missing boolean input.");
    setLogicBranchToExecute(-1);
    return;
  }

  if (in_bool->value())
  {
    _out_ports[0].out_value = std::make_shared<LogicData>(true);
    _out_ports[1].out_value = std::make_shared<LogicData>(false);
    setLogicBranchToExecute(0);
  }
  else
  {
    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _out_ports[1].out_value = std::make_shared<LogicData>(true);
    setLogicBranchToExecute(1);
  }

  Q_EMIT dataUpdated(0);
  Q_EMIT dataUpdated(1);

  setValidationState(NodeValidationState::Warning);
  setValidationMessage(in_bool->value() ? "Debug: true" : "Debug: false");
}

NodeValidationState LogicIfNode::validate()
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

    return _validation_state;
  }

  auto in_bool = _in_ports[1].in_value.lock();
  auto in_bool_ptr = static_cast<BooleanData*>(in_bool.get());

  if (!in_bool_ptr)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Missing boolean input.");
    return _validation_state;
  }

  return _validation_state;
}


