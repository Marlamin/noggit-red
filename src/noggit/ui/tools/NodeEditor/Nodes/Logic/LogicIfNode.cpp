#include "LogicIfNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

LogicIfNode::LogicIfNode()
: LogicNodeBase()
{
  setName("Logic :: If");
  setCaption("Logic :: If");
  setValidationState(NodeValidationState::Valid);

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<BooleanData>(PortType::In, "Boolean", true);
  addPort<LogicData>(PortType::Out, "True", true, ConnectionPolicy::One);
  addPort<LogicData>(PortType::Out, "False", true, ConnectionPolicy::One);
}

void LogicIfNode::compute()
{

  if (static_cast<BooleanData*>(_in_ports[1].in_value.lock().get())->value())
  {
    _out_ports[0].out_value = std::make_shared<LogicData>(true);
    _out_ports[1].out_value = std::make_shared<LogicData>(false);
  }
  else
  {
    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _out_ports[1].out_value = std::make_shared<LogicData>(true);
  }

  _node->onDataUpdated(0);
  _node->onDataUpdated(1);
}

NodeValidationState LogicIfNode::validate()
{
  if (!static_cast<LogicData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _out_ports[1].out_value = std::make_shared<LogicData>(false);
    _node->onDataUpdated(0);
    _node->onDataUpdated(1);

    return _validation_state;
  }

  if (!static_cast<BooleanData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate boolean input.\"");
    return _validation_state;
  }

  return _validation_state;
}
