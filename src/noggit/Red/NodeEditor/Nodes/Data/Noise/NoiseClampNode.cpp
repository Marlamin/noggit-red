// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseClampNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseClampNode::NoiseClampNode()
: BaseNode()
{
  setName("NoiseClampNode");
  setCaption("Noise Clamp");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<Vector2DData>(PortType::In, "Bounds<Vector2D>", true);
  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseClampNode::compute()
{
  auto module = new noise::module::Clamp();
  module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());
  glm::vec2 bounds = defaultPortData<Vector2DData>(PortType::In, 1)->value();
  module->SetBounds(bounds.x, bounds.y);

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);
}

NodeValidationState NoiseClampNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}


QJsonObject NoiseClampNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "bounds");

  return json_obj;
}

void NoiseClampNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "bounds");
}

