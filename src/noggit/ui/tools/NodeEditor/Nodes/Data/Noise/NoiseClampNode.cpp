// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseClampNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseClampNode::NoiseClampNode()
: BaseNode()
{
  setName("Noise :: Clamp");
  setCaption("Noise :: Clamp");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<Vector2DData>(PortType::In, "Bounds<Vector2D>", true);
  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseClampNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());

  auto bounds_data = defaultPortData<Vector2DData>(PortType::In, 1);

  glm::vec2 const& bounds = bounds_data->value();

  if (bounds.x >= bounds.y)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: incorrect bounds.");
    return;
  }

  _module.SetBounds(bounds.x, bounds.y);

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);

  _node->onDataUpdated(0);
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
