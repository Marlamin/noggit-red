// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListReserveNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ListReserveNode::ListReserveNode()
: LogicNodeBase()
{
  setName("List :: Reserve");
  setCaption("List :: Reserve");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPort<ListData>(PortType::In, "List[Any]", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 1);
  addPort<UnsignedIntegerData>(PortType::In, "nElements<UInteger>", true);
  addDefaultWidget(_in_ports[2].data_type->default_widget(&_embedded_widget), PortType::In, 2);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

void ListReserveNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
    return;

  auto n_elements_ptr = static_cast<UnsignedIntegerData*>(_in_ports[2].in_value.lock().get());
  list->value()->reserve(n_elements_ptr ? n_elements_ptr->value() : static_cast<QSpinBox*>(_in_ports[2].default_widget)->value());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState ListReserveNode::validate()
{
  LogicNodeBase::validate();

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate list input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    _node->onDataUpdated(0);
  }

  return _validation_state;
}

QJsonObject ListReserveNode::save() const
{
  QJsonObject json_obj = BaseNode::save();
  _in_ports[2].data_type->to_json(_in_ports[2].default_widget, json_obj,  "n_elements");

  return json_obj;
}

void ListReserveNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  _in_ports[2].data_type->from_json(_in_ports[2].default_widget, json_obj,  "n_elements");
}
