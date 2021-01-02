// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListDeclareNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

ListDeclareNode::ListDeclareNode()
: ListNodeBase()
{
  setName("ListDeclareNode");
  setCaption("List Declare");
  _validation_state = NodeValidationState::Valid;

  addPort<ListData>(PortType::In, "List[Integer]", true);
  addPort<ListData>(PortType::Out, "List[Integer]", true);
  _out_ports[0].data_type->set_parameter_type("int");

  _type = new QComboBox(&_embedded_widget);
  _type->addItems(_type_list);

  QComboBox::connect(_type, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
       auto new_type_id = _type_map[_type->currentText().toStdString()].c_str();

       if (_out_ports[0].data_type->type().parameter_type_id == new_type_id)
         return;

       deletePort(PortType::Out, 0);

       addPortDynamic<ListData>(PortType::Out, 0, "List[" + _type->currentText() + "]", true);
       _out_ports[0].data_type->set_parameter_type(new_type_id);

       deletePort(PortType::In, 0);

       addPortDynamic<ListData>(PortType::In, 0, "List[" + _type->currentText() + "]", true);
       _in_ports[0].data_type->set_parameter_type(new_type_id);

     }
  );

  addWidgetTop(_type);
}

void ListDeclareNode::compute()
{
  auto list = static_cast<ListData*>(_in_ports[0].in_value.lock().get());

  auto parameter_id = list->type().parameter_type_id;
  if (parameter_id.isEmpty())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to resolve list parameter type.");
    return;
  }

  if (parameter_id != _type_map.at(_type->currentText().toStdString()).c_str())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: passed list does not match declared type.");
    return;
  }

  _out_ports[0].out_value = _in_ports[0].in_value.lock();
  Q_EMIT dataUpdated(0);

}

QJsonObject ListDeclareNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["type"] = _type->currentText();

  return json_obj;
}

void ListDeclareNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  auto type_name = json_obj["type"].toString();
  auto new_type_id = _type_map[type_name.toStdString()].c_str();
  _out_ports[0].data_type->set_parameter_type(new_type_id);

  _type->setCurrentText(type_name);

}

NodeValidationState ListDeclareNode::validate()
{
  auto list_ptr = static_cast<ListData*>(_in_ports[0].in_value.lock().get());

  if (!list_ptr)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate list input.");
    return _validation_state;
  }

  return _validation_state;
}

