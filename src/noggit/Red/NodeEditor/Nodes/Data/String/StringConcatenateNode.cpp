// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "StringConcatenateNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

StringConcatenateNode::StringConcatenateNode()
: BaseNode()
{
  setName("String :: Concatenate");
  setCaption("String :: Concatenate");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<StringData>(PortType::In, "String", true);
  addPortDefault<StringData>(PortType::In, "String", true);

  addPort<StringData>(PortType::Out, "String", true);
}

void StringConcatenateNode::compute()
{
  _out_ports[0].out_value = std::make_shared<StringData>(defaultPortData<StringData>(PortType::In, 0)->value()
      + defaultPortData<StringData>(PortType::In, 1)->value());

  _node->onDataUpdated(0);
}

QJsonObject StringConcatenateNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 0, json_obj, "first");
  defaultWidgetToJson(PortType::In, 1, json_obj, "second");

  return json_obj;
}

void StringConcatenateNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 0, json_obj, "first");
  defaultWidgetFromJson(PortType::In, 1, json_obj, "second");
}


