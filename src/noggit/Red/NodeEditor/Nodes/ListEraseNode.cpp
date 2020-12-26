// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListEraseNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

ListEraseNode::ListEraseNode()
    : LogicNodeBase()
{
  setName("ListEraseNode");
  setCaption("List Erase");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPort<ListData>(PortType::In, "List[Any]", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 1);
  addPort<UnsignedIntegerData>(PortType::In, "Index<UInteger>", true);
  addDefaultWidget(_in_ports[2].data_type->default_widget(&_embedded_widget), PortType::In, 2);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

void ListEraseNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
    return;

  auto list_obj = list->value();

  auto index_ptr = static_cast<UnsignedIntegerData*>(_in_ports[2].in_value.lock().get());
  auto index = (index_ptr ? index_ptr->value() : static_cast<QSpinBox*>(_in_ports[2].default_widget)->value());

  if (index < 0 || index >= list_obj->size())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: invalid index");
    return;
  }

  list_obj->erase(list_obj->begin() + index);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

}

NodeValidationState ListEraseNode::validate()
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

QJsonObject ListEraseNode::save() const
{
  QJsonObject json_obj = BaseNode::save();
  _in_ports[2].data_type->to_json(_in_ports[2].default_widget, json_obj,  "index");

  return json_obj;
}

void ListEraseNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  _in_ports[2].data_type->from_json(_in_ports[2].default_widget, json_obj,  "index");
}

