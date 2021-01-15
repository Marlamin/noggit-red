// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RandomDecimalNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <QRandomGenerator>

using namespace noggit::Red::NodeEditor::Nodes;

RandomDecimalNode::RandomDecimalNode()
: BaseNode()
{
  setName("RandomDecimalNode");
  setCaption("Random Decimal");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<IntegerData>(PortType::In, "Seed", true);
  addPort<DecimalData>(PortType::Out, "Decimal", true);
}

void RandomDecimalNode::compute()
{
  QRandomGenerator rand;
  rand.seed(defaultPortData<IntegerData>(PortType::In, 0)->value());

  _out_ports[0].out_value = std::make_shared<DecimalData>(rand.generateDouble());

  Q_EMIT dataUpdated(0);
}

QJsonObject RandomDecimalNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 0, json_obj, "seed");

  return json_obj;
}

void RandomDecimalNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 0, json_obj, "seed");
}

