// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RandomIntegerRangeNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <QRandomGenerator>

using namespace noggit::ui::tools::NodeEditor::Nodes;

RandomIntegerRangeNode::RandomIntegerRangeNode()
: BaseNode()
{
  setName("Random :: IntegerRange");
  setCaption("Random :: IntegerRange");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<IntegerData>(PortType::In, "Seed", true);
  addPortDefault<IntegerData>(PortType::In, "MinInclusive<Integer>", true);
  addPortDefault<IntegerData>(PortType::In, "MaxExclusive<Integer>", true);
  addPort<IntegerData>(PortType::Out, "Integer", true);
}

void RandomIntegerRangeNode::compute()
{
  QRandomGenerator rand;
  rand.seed(defaultPortData<IntegerData>(PortType::In, 0)->value());

  int lowest = defaultPortData<IntegerData>(PortType::In, 1)->value();
  int highest =  defaultPortData<IntegerData>(PortType::In, 2)->value();

  if (lowest >= highest)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: incorrect range.");
    return;
  }
  _out_ports[0].out_value = std::make_shared<IntegerData>(rand.bounded(lowest, highest));

  _node->onDataUpdated(0);
}

QJsonObject RandomIntegerRangeNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 0, json_obj, "seed");
  defaultWidgetToJson(PortType::In, 1, json_obj, "min_inclusive");
  defaultWidgetToJson(PortType::In, 2, json_obj, "max_exclusive");

  return json_obj;
}

void RandomIntegerRangeNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 0, json_obj, "seed");
  defaultWidgetFromJson(PortType::In, 1, json_obj, "min_inclusive");
  defaultWidgetFromJson(PortType::In, 2, json_obj, "max_exclusive");
}
