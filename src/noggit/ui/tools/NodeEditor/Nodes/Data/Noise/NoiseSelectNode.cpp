// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseSelectNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseSelectNode::NoiseSelectNode()
: BaseNode()
{
  setName("Noise :: Select");
  setCaption("Noise :: Select");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<NoiseData>(PortType::In, "Control<Noise>", true);

  addPortDefault<DecimalData>(PortType::In, "EdgeFalloff<Decimal>", true);
  auto falloff = static_cast<QDoubleSpinBox*>(_in_ports[3].default_widget);
  falloff->setValue(1.0);

  addPortDefault<Vector2DData>(PortType::In, "Bounds<Vector2D>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseSelectNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());
  _module.SetSourceModule(1, *static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())->value());
  _module.SetControlModule(*static_cast<NoiseData*>(_in_ports[2].in_value.lock().get())->value());
  _module.SetEdgeFalloff(defaultPortData<DecimalData>(PortType::In, 3)->value());

  glm::vec2 bounds = defaultPortData<Vector2DData>(PortType::In, 4)->value();

  if (bounds.x >= bounds.y)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: bad bounds");
    return;
  }

  _module.SetBounds(bounds.x, bounds.y);
  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);
}

QJsonObject NoiseSelectNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 3, json_obj, "edge_falloff");
  defaultWidgetToJson(PortType::In, 4, json_obj, "bounds");

  return json_obj;
}

void NoiseSelectNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  defaultWidgetFromJson(PortType::In, 3, json_obj, "edge_falloff");
  defaultWidgetFromJson(PortType::In, 4, json_obj, "bounds");

}


NodeValidationState NoiseSelectNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())
      || !static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())
      || !static_cast<NoiseData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
