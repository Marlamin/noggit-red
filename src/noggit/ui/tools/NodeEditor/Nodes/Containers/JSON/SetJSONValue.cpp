// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SetJSONValue.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

SetJSONValueNode::SetJSONValueNode()
: LogicNodeBase()
{
  setName("JSON :: SetJSONValue");
  setCaption("JSON :: SetJSONValue");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<JSONData>(PortType::In, "JSONObject", true);
  addPortDefault<StringData>(PortType::In, "Name<String>", true);
  addPortDefault<JSONValueData>(PortType::In, "JSONValue", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SetJSONValueNode::compute()
{
  auto var_name_data = defaultPortData<StringData>(PortType::In,  2);
  std::string const& var_name = var_name_data->value();
  QJsonValue* json_val = defaultPortData<JSONValueData>(PortType::In,  3)->value_ptr();
  QJsonObject* json_obj = defaultPortData<JSONData>(PortType::In,  1)->value_ptr();

  (*json_obj)[var_name.c_str()] = *json_val;

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState SetJSONValueNode::validate()
{
  if (!static_cast<JSONData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json input.");
    return _validation_state;
  }

  if (!static_cast<JSONValueData*>(_in_ports[3].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json value input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject SetJSONValueNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "var_name");

  return json_obj;
}

void SetJSONValueNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "var_name");
}

