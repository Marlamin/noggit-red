// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "JSONArrayPush.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

JSONArrayPushNode::JSONArrayPushNode()
: LogicNodeBase()
{
  setName("JSON:: JSONArrayPush");
  setCaption("JSON :: JSONArrayPush");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Back", "Front"});
  addWidgetTop(_operation);

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<JSONArrayData>(PortType::In, "JSONArray", true);
  addPort<JSONValueData>(PortType::In, "JSONValue", true);
}

void JSONArrayPushNode::compute()
{
  QJsonArray* json_array = defaultPortData<JSONArrayData>(PortType::In, 1)->value_ptr();
  QJsonValue* json_value = defaultPortData<JSONValueData>(PortType::In, 2)->value_ptr();

  switch (_operation->currentIndex())
  {
    case 0: // Back
      json_array->push_back(*json_value);
      break;
    case 1: // Front
      json_array->push_front(*json_value);
      break;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState JSONArrayPushNode::validate()
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

QJsonObject JSONArrayPushNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void JSONArrayPushNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  _operation->setCurrentIndex(json_obj["operation"].toInt());
}

