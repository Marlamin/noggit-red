// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListSizeNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ListSizeNode::ListSizeNode()
: LogicNodeBase()
{
  setName("List :: Size");
  setCaption("List :: Size");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<ListData>(PortType::In, "List[Any]", true);
  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
  addPort<UnsignedIntegerData>(PortType::Out, "Size<UInteger>", true);
}

void ListSizeNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
    return;

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<UnsignedIntegerData>(list->value()->size());
  Q_EMIT dataUpdated(1);

}

NodeValidationState ListSizeNode::validate()
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
