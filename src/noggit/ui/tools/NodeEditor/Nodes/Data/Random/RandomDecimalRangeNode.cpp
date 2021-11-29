// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RandomDecimalRangeNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <QRandomGenerator>

using namespace noggit::Red::NodeEditor::Nodes;

RandomDecimalRangeNode::RandomDecimalRangeNode()
: BaseNode()
{
  setName("Random :: DecimalRange");
  setCaption("Random :: DecimalRange");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<IntegerData>(PortType::In, "Seed", true);
  addPortDefault<DecimalData>(PortType::In, "Highest<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "Decimal", true);
}

void RandomDecimalRangeNode::compute()
{
  QRandomGenerator rand;
  rand.seed(defaultPortData<DecimalData>(PortType::In, 0)->value());
  double highest = defaultPortData<DecimalData>(PortType::In, 1)->value();

  _out_ports[0].out_value = std::make_shared<DecimalData>(rand.bounded(highest));

  _node->onDataUpdated(0);
}

QJsonObject RandomDecimalRangeNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 0, json_obj, "seed");
  defaultWidgetToJson(PortType::In, 1, json_obj, "highest");

  return json_obj;
}

void RandomDecimalRangeNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 0, json_obj, "seed");
  defaultWidgetFromJson(PortType::In, 1, json_obj, "highest");
}
