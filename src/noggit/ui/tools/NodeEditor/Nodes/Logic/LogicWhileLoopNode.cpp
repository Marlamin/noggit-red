// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicWhileLoopNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

LogicWhileLoopNode::LogicWhileLoopNode()
: LogicNodeBase()
{
  setName("Logic :: WhileLoop");
  setCaption("Logic :: WhileLoop");
  _validation_state = NodeValidationState::Valid;
  setInterpreterToken(NodeInterpreterTokens::WHILE);

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<BooleanData>(PortType::In, "Boolean", true);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);

  setIsIterable(true);
}

void LogicWhileLoopNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if(!logic->value())
    return;

  auto in_bool = static_cast<BooleanData*>(_in_ports[1].in_value.lock().get());

  if (!in_bool->value())
  {
    setIterationIndex(-1);
    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _node->onDataUpdated(0);
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState LogicWhileLoopNode::validate()
{
  setValidationState(NodeValidationState::Valid);

  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _node->onDataUpdated(0);
  }

  auto in_bool = static_cast<BooleanData*>(_in_ports[1].in_value.lock().get());

  if (!in_bool)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate boolean input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _node->onDataUpdated(0);
  }

  if (!in_bool->value())
  {
    setIterationIndex(-1);
  }

  setIterationIndex(0);
  setNIterations(1);

  return _validation_state;
}
