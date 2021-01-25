// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseTurbulenceNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseTurbulenceNode::NoiseTurbulenceNode()
: BaseNode()
{
  setName("Noise :: Turbulence");
  setCaption("Noise :: Turbulence");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);

  addPortDefault<DecimalData>(PortType::In, "Frequency<Decimal>", true);
  auto frequency = static_cast<QDoubleSpinBox*>(_in_ports[1].default_widget);
  frequency->setMinimum(0.0);
  frequency->setValue(1.0);

  addPortDefault<DecimalData>(PortType::In, "Power<Decimal>", true);
  auto power = static_cast<QDoubleSpinBox*>(_in_ports[2].default_widget);
  power->setMinimum(0.0);
  power->setValue(1.0);

  addPortDefault<IntegerData>(PortType::In, "Roughness<Integer>", true);
  auto roughness = static_cast<QSpinBox*>(_in_ports[3].default_widget);
  roughness->setMinimum(1);
  roughness->setValue(3);

  addPortDefault<IntegerData>(PortType::In, "Seed<Integer>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseTurbulenceNode::compute()
{
  auto module = new noise::module::Turbulence();
  module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());

  module->SetFrequency(defaultPortData<DecimalData>(PortType::In, 1)->value());
  module->SetPower(defaultPortData<DecimalData>(PortType::In, 2)->value());
  module->SetRoughness(std::min(31, std::max(1, defaultPortData<IntegerData>(PortType::In, 3)->value())));
  module->SetSeed(defaultPortData<IntegerData>(PortType::In, 4)->value());

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  _node->onDataUpdated(0);
}

QJsonObject NoiseTurbulenceNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "frequency");
  defaultWidgetToJson(PortType::In, 2, json_obj, "power");
  defaultWidgetToJson(PortType::In, 3, json_obj, "roughness");
  defaultWidgetToJson(PortType::In, 4, json_obj, "seed");

  return json_obj;
}

void NoiseTurbulenceNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "frequency");
  defaultWidgetFromJson(PortType::In, 2, json_obj, "power");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "roughness");
  defaultWidgetFromJson(PortType::In, 4, json_obj, "seed");
}


NodeValidationState NoiseTurbulenceNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
