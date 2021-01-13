// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseScaleBiasNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseScaleBiasNode::NoiseScaleBiasNode()
: BaseNode()
{
  setName("NoiseScaleBiasNode");
  setCaption("Noise Scale Bias");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<DecimalData>(PortType::In, "Bias<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "Scale<Decimal>", true);
  auto scale = static_cast<QDoubleSpinBox*>(_in_ports[2].default_widget);
  scale->setValue(1.0);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseScaleBiasNode::compute()
{
  auto module = new noise::module::ScaleBias();
  module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());

  module->SetBias(defaultPortData<DecimalData>(PortType::In, 1)->value());
  module->SetScale(defaultPortData<DecimalData>(PortType::In, 2)->value());

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);
}

QJsonObject NoiseScaleBiasNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "bias");
  defaultWidgetToJson(PortType::In, 2, json_obj, "scale");

  return json_obj;
}

void NoiseScaleBiasNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  defaultWidgetFromJson(PortType::In, 1, json_obj, "bias");
  defaultWidgetFromJson(PortType::In, 2, json_obj, "scale");
}


NodeValidationState NoiseScaleBiasNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}


