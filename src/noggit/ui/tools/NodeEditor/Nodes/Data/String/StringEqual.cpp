// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "StringEqual.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

StringEqualNode::StringEqualNode()
: BaseNode()
{
  setName("String :: Equal");
  setCaption("String :: Equal");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<StringData>(PortType::In, "String", true);
  addPortDefault<StringData>(PortType::In, "String", true);

  addPort<BooleanData>(PortType::Out, "String", true);
}

void StringEqualNode::compute()
{
  auto first = defaultPortData<StringData>(PortType::In, 0)->value();
  auto second = defaultPortData<StringData>(PortType::In, 1)->value();

  _out_ports[0].out_value = std::make_shared<BooleanData>(first == second);
  _node->onDataUpdated(0);
}

QJsonObject StringEqualNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 0, json_obj, "first");
  defaultWidgetToJson(PortType::In, 0, json_obj, "second");

  return json_obj;
}

void StringEqualNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 0, json_obj, "first");
  defaultWidgetFromJson(PortType::In, 0, json_obj, "second");
}

