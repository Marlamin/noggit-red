// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Vector2DToXYNode.hpp"


#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

Vector2DToXYNode::Vector2DToXYNode()
    : BaseNode()
{
  setName("Vector :: Vector2DToXY");
  setCaption("Vector :: Vector2DToXY");
  _validation_state = NodeValidationState::Valid;

  addPort<Vector2DData>(PortType::In, "Vector2D", true);

  addPort<DecimalData>(PortType::Out, "X<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "Y<Decimal>", true);
}

void Vector2DToXYNode::compute()
{
  glm::vec2 vector = static_cast<Vector2DData*>(_in_ports[0].in_value.lock().get())->value();
  _out_ports[0].out_value = std::make_shared<DecimalData>(vector.x);
  _out_ports[1].out_value = std::make_shared<DecimalData>(vector.y);

  _node->onDataUpdated(0);
  _node->onDataUpdated(1);
}

NodeValidationState Vector2DToXYNode::validate()
{

  if (!static_cast<Vector2DData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate vector input.");
    return _validation_state;
  }

  return _validation_state;
}
