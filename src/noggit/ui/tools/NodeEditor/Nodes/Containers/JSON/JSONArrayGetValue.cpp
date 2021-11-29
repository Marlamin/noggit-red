// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "JSONArrayGetValue.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

JSONArrayGetValueNode::JSONArrayGetValueNode()
: BaseNode()
{
  setName("JSON :: JsonArrayGetValue");
  setCaption("JSON :: JsonArrayGetValue");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<JSONArrayData>(PortType::In, "JSONArray", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Index<UInteger>", true);

  addPort<StringData>(PortType::Out, "String", true);
  addPort<BooleanData>(PortType::Out, "Boolean", true);
  addPort<DecimalData>(PortType::Out, "Decimal", true);
  addPort<IntegerData>(PortType::Out, "Integer", true);
  addPort<JSONData>(PortType::Out, "JSONObject", true);
  addPort<JSONArrayData>(PortType::Out, "JSONArray", true);
}

void JSONArrayGetValueNode::compute()
{
  QJsonArray* json_array = defaultPortData<JSONArrayData>(PortType::In, 0)->value_ptr();
  unsigned index = defaultPortData<UnsignedIntegerData>(PortType::In, 1)->value();

  QJsonValue value = json_array->at(index);

  if (value.isUndefined())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: value is undefined. Possibly out of range index.");
    return;
  }

  if (_out_ports[0].connected)
  {
    if (!value.isString())
      goto _ERROR;

    _out_ports[0].out_value = std::make_shared<StringData>(value.toString().toStdString());
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    if (!value.isBool())
      goto _ERROR;

    _out_ports[1].out_value = std::make_shared<BooleanData>(value.toBool());
    _node->onDataUpdated(1);
  }

  if (_out_ports[2].connected)
  {
    if (!value.isDouble())
      goto _ERROR;

    _out_ports[2].out_value = std::make_shared<DecimalData>(value.toDouble());
    _node->onDataUpdated(2);
  }

  if (_out_ports[3].connected)
  {
    if (!value.isDouble())
      goto _ERROR;

    _out_ports[3].out_value = std::make_shared<DecimalData>(value.toInt());
    _node->onDataUpdated(3);
  }

  if (_out_ports[4].connected)
  {
    if (!value.isObject())
      goto _ERROR;

    _out_ports[4].out_value = std::make_shared<JSONData>(value.toObject());
    _node->onDataUpdated(4);
  }

  if (_out_ports[5].connected)
  {
    if (!value.isArray())
      goto _ERROR;

    _out_ports[5].out_value = std::make_shared<JSONArrayData>(value.toArray());
    _node->onDataUpdated(5);
  }

  return;

  _ERROR:
  setValidationState(NodeValidationState::Error);
  setValidationMessage("Error: type mismatch.");
}

NodeValidationState JSONArrayGetValueNode::validate()
{
  if (!static_cast<JSONArrayData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json array input.");
    return _validation_state;
  }

  return _validation_state;
}

QJsonObject JSONArrayGetValueNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "index");

  return json_obj;
}

void JSONArrayGetValueNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "index");
}

