// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicChainNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

#include <vector>

using namespace noggit::Red::PresetEditor::Nodes;

LogicChainNode::LogicChainNode()
: LogicNodeBase()
{
  setName("LogicChainNode");
  setCaption("Chain");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

void LogicChainNode::compute()
{
  int count = 0;
  for (auto& port : _out_ports)
  {
    _out_ports[count].out_value = std::make_shared<LogicData>(true);
    count++;
  }
}

NodeValidationState LogicChainNode::validate()
{
  return BaseNode::validate();
}

void LogicChainNode::outputConnectionCreated(const Connection& connection)
{
  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

