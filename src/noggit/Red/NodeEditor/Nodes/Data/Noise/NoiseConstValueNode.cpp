// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseConstValueNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseConstValueNode::NoiseConstValueNode()
: NoiseGeneratorBase()
{
  setName("NoiseConstValueNode");
  setCaption("Const Value Noise");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<DecimalData>(PortType::In, "Value<Decimal>", true);
  auto value = static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget);
  value->setMinimum(0.0);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseConstValueNode::compute()
{
  auto module = new noise::module::Const();

  double value = defaultPortData<DecimalData>(PortType::In, 0)->value();
  if (!checkBounds(value, 0.0, std::numeric_limits<double>::max(), "Value"))
    return;
  module->SetConstValue(value);

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);
}
