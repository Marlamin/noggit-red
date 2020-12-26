// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListClearNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

ListClearNode::ListClearNode()
: LogicNodeBase()
{
  setName("ListSizeNode");
  setCaption("List Size");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<ListData>(PortType::In, "List[Any]", true);
  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

void ListClearNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
    return;

  list->value()->clear();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

}

NodeValidationState ListClearNode::validate()
{
  LogicNodeBase::validate();

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate list input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    Q_EMIT dataUpdated(0);
  }

  return _validation_state;
}
