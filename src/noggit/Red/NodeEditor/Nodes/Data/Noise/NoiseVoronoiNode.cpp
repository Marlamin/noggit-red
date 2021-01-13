// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseVoronoiNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseVoronoiNode::NoiseVoronoiNode()
: NoiseGeneratorBase()
{
  setName("NoiseVoronoiNode");
  setCaption("Voronoi Noise");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<DecimalData>(PortType::In, "Frequency<Decimal>", true);
  auto frequency = static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget);
  frequency->setMinimum(0.0);
  frequency->setValue(1.0);

  addPortDefault<DecimalData>(PortType::In, "Displacement<Decimal>", true);
  auto lacunarity = static_cast<QDoubleSpinBox*>(_in_ports[1].default_widget);
  lacunarity->setMinimum(0.0);
  lacunarity->setValue(1.0);

  addPortDefault<UnsignedIntegerData>(PortType::In, "Seed<Integer>", true);
  addPortDefault<BooleanData>(PortType::In, "UseDistance<Boolean>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseVoronoiNode::compute()
{
  auto module = new noise::module::Voronoi();

  double frequency = defaultPortData<DecimalData>(PortType::In, 0)->value();
  if (!checkBounds(frequency, 0.0, std::numeric_limits<double>::max(), "Frequency"))
    return;
  module->SetFrequency(frequency);

  double displacement = defaultPortData<DecimalData>(PortType::In, 1)->value();
  if (!checkBounds(displacement, 0.0, std::numeric_limits<double>::max(), "Lacunarity"))
    return;
  module->SetDisplacement(displacement);

  module->SetSeed(defaultPortData<IntegerData>(PortType::In, 2)->value());
  module->EnableDistance(defaultPortData<BooleanData>(PortType::In, 3)->value());

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);
}


