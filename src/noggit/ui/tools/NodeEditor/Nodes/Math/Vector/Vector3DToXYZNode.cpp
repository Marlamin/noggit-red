// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Vector3DToXYZNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

Vector3DToXYZNode::Vector3DToXYZNode()
: BaseNode()
{
  setName("Vector :: Vector3DToXYZ");
  setCaption("Vector :: Vector3DToXYZ");
  _validation_state = NodeValidationState::Valid;

  addPort<Vector3DData>(PortType::In, "Vector3D", true);

  addPort<DecimalData>(PortType::Out, "X<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "Y<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "Z<Decimal>", true);
}

void Vector3DToXYZNode::compute()
{
  glm::vec3 vector = static_cast<Vector3DData*>(_in_ports[0].in_value.lock().get())->value();
  _out_ports[0].out_value = std::make_shared<DecimalData>(vector.x);
  _out_ports[1].out_value = std::make_shared<DecimalData>(vector.y);
  _out_ports[2].out_value = std::make_shared<DecimalData>(vector.z);

  _node->onDataUpdated(0);
  _node->onDataUpdated(1);
  _node->onDataUpdated(2);
}

NodeValidationState Vector3DToXYZNode::validate()
{

  if (!static_cast<Vector3DData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate vector input.");
    return _validation_state;
  }

  return _validation_state;
}
