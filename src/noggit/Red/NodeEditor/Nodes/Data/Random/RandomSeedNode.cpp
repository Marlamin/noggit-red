// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RandomSeedNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <random>
#include <limits>

using namespace noggit::Red::NodeEditor::Nodes;

RandomSeedNode::RandomSeedNode()
: LogicNodeBase()
{
  setName("Random :: Seed");
  setCaption("Random :: Seed");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<IntegerData>(PortType::Out, "Seed", true);
}

void RandomSeedNode::compute()
{
  std::random_device r;

  std::default_random_engine e1(r());
  std::uniform_int_distribution<int> uniform_dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);
  std::srand(time(nullptr));
  _out_ports[1].out_value = std::make_shared<IntegerData>(uniform_dist(e1));
  Q_EMIT dataUpdated(1);
}
