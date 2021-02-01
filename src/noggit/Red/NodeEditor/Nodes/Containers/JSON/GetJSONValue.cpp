// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetJSONValue.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

GetJSONValueNode::GetJSONValueNode()
: BaseNode()
{
  setName("JSON :: GetJSONValue");
  setCaption("JSON :: GetJSONValue");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<JSONData>(PortType::In, "JSONObject", true);
  addPortDefault<StringData>(PortType::In, "Name<String>", true);

  addPort<StringData>(PortType::Out, "String", true);
  addPort<BooleanData>(PortType::Out, "Boolean", true);
  addPort<DecimalData>(PortType::Out, "Decimal", true);
  addPort<IntegerData>(PortType::Out, "Integer", true);
  addPort<JSONData>(PortType::Out, "JSONObject", true);
  addPort<JSONArrayData>(PortType::Out, "JSONArray", true);

}

void GetJSONValueNode::compute()
{
  QJsonObject* json_obj = static_cast<JSONData*>(_in_ports[0].in_value.lock().get())->value_ptr();
  std::string const& name = defaultPortData<StringData>(PortType::In, 1)->value();
  QJsonValueRef value = (*json_obj)[name.c_str()];

  if (value.isUndefined())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: value is undefined. Possibly non-existing key.");
    return;
  }

  if (_out_ports[0].connected)
  {
    if (!value.isString())
      goto ERROR;

    _out_ports[0].out_value = std::make_shared<StringData>(value.toString().toStdString());
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    if (!value.isBool())
      goto ERROR;

    _out_ports[1].out_value = std::make_shared<BooleanData>(value.toBool());
    _node->onDataUpdated(1);
  }

  if (_out_ports[2].connected)
  {
    if (!value.isDouble())
      goto ERROR;

    _out_ports[2].out_value = std::make_shared<DecimalData>(value.toDouble());
    _node->onDataUpdated(2);
  }

  if (_out_ports[3].connected)
  {
    if (!value.isDouble())
      goto ERROR;

    _out_ports[3].out_value = std::make_shared<DecimalData>(value.toInt());
    _node->onDataUpdated(3);
  }

  if (_out_ports[4].connected)
  {
    if (!value.isObject())
      goto ERROR;

    _out_ports[4].out_value = std::make_shared<JSONData>(value.toObject());
    _node->onDataUpdated(4);
  }

  if (_out_ports[5].connected)
  {
    if (!value.isArray())
      goto ERROR;

    _out_ports[5].out_value = std::make_shared<JSONArrayData>(value.toArray());
    _node->onDataUpdated(5);
  }

  return;

  ERROR:
  setValidationState(NodeValidationState::Error);
  setValidationMessage("Error: type mismatch.");
  return;

}

QJsonObject GetJSONValueNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "var_name");

  return json_obj;
}

void GetJSONValueNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "var_name");
}


NodeValidationState GetJSONValueNode::validate()
{
  if (!static_cast<JSONData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json input.");
    return _validation_state;
  }

  return _validation_state;
}