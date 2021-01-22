// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Vector4DToXYZWNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

Vector4DToXYZWNode::Vector4DToXYZWNode()
    : BaseNode()
{
  setName("Vector :: Vector4DToXYZW");
  setCaption("Vector :: Vector4DToXYZW");
  _validation_state = NodeValidationState::Valid;

  addPort<Vector4DData>(PortType::In, "Vector4D", true);

  addPort<DecimalData>(PortType::Out, "X<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "Y<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "Z<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "W<Decimal>", true);
}

void Vector4DToXYZWNode::compute()
{
  glm::vec4 vector = static_cast<Vector4DData*>(_in_ports[0].in_value.lock().get())->value();
  _out_ports[0].out_value = std::make_shared<DecimalData>(vector.x);
  _out_ports[1].out_value = std::make_shared<DecimalData>(vector.y);
  _out_ports[2].out_value = std::make_shared<DecimalData>(vector.z);
  _out_ports[3].out_value = std::make_shared<DecimalData>(vector.w);

  Q_EMIT dataUpdated(0);
  Q_EMIT dataUpdated(1);
  Q_EMIT dataUpdated(2);
  Q_EMIT dataUpdated(3);
}

NodeValidationState Vector4DToXYZWNode::validate()
{

  if (!static_cast<Vector4DData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate vector input.");
    return _validation_state;
  }

  return _validation_state;
}
