// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseExponentNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseExponentNode::NoiseExponentNode()
: BaseNode()
{
  setName("Noise :: Exponent");
  setCaption("Noise :: Exponent");
  _validation_state = NodeValidationState::Valid;


  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<DecimalData>(PortType::In, "Exponent<Decimal>", true);
  auto exponent = static_cast<QDoubleSpinBox*>(_in_ports[1].default_widget);
  exponent->setValue(1.0);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseExponentNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());

  _module.SetExponent(defaultPortData<DecimalData>(PortType::In, 1)->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);
}

QJsonObject NoiseExponentNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "exponent");

  return json_obj;
}

void NoiseExponentNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "exponent");
}


NodeValidationState NoiseExponentNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
