// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "JSONArrayInsertValue.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

JSONArrayInsertValueNode::JSONArrayInsertValueNode()
: LogicNodeBase()
{
  setName("JSON :: JSONArrayInsertValue");
  setCaption("JSON :: JSONArrayInsertValue");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<JSONArrayData>(PortType::In, "JSONArray", true);
  addPortDefault<JSONValueData>(PortType::In, "JSONValue", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Index<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void JSONArrayInsertValueNode::compute()
{
  QJsonArray* json_array = defaultPortData<JSONArrayData>(PortType::In, 1)->value_ptr();
  QJsonValue* json_val = defaultPortData<JSONValueData>(PortType::In, 2)->value_ptr();
  int index = static_cast<int>(defaultPortData<UnsignedIntegerData>(PortType::In, 3)->value());

  if (index > json_array->size() || index < 0)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: index is out of range.");
    return;
  }

  json_array->insert(index, *json_val);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState JSONArrayInsertValueNode::validate()
{
  if (!static_cast<JSONArrayData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json array input.");
    return _validation_state;
  }

  if (!static_cast<JSONValueData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json value input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject JSONArrayInsertValueNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 3, json_obj, "index");

  return json_obj;
}

void JSONArrayInsertValueNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 3, json_obj, "index");
}

