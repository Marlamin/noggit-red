// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RandomIntegerNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <QRandomGenerator>

using namespace noggit::Red::NodeEditor::Nodes;

RandomIntegerNode::RandomIntegerNode()
: BaseNode()
{
  setName("Random :: Integer");
  setCaption("Random :: Integer");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<IntegerData>(PortType::In, "Seed", true);
  addPort<IntegerData>(PortType::Out, "Integer", true);
}

void RandomIntegerNode::compute()
{
  QRandomGenerator rand;
  rand.seed(defaultPortData<IntegerData>(PortType::In, 0)->value());

  _out_ports[0].out_value = std::make_shared<IntegerData>(rand.generate());

  Q_EMIT dataUpdated(0);
}

QJsonObject RandomIntegerNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 0, json_obj, "seed");

  return json_obj;
}

void RandomIntegerNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 0, json_obj, "seed");
}
