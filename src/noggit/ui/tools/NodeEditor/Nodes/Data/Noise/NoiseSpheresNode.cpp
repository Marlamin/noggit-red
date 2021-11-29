// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseSpheresNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseSpheresNode::NoiseSpheresNode()
: NoiseGeneratorBase()
{
  setName("Noise :: Spheres");
  setCaption("Noise :: Spheres");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<DecimalData>(PortType::In, "Frequency<Decimal>", true);
  auto frequency = static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget);
  frequency->setMinimum(0.0);
  frequency->setValue(1.0);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseSpheresNode::compute()
{
  double frequency = defaultPortData<DecimalData>(PortType::In, 0)->value();
  if (!checkBounds(frequency, 0.0, std::numeric_limits<double>::max(), "Frequency"))
    return;
  _module.SetFrequency(frequency);

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);
}
