// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "XYZtoVector3DNode.hpp"


#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

XYZtoVector3DNode::XYZtoVector3DNode()
    : BaseNode()
{
  setName("Vector :: XYZtoVector3D");
  setCaption("Vector :: XYZtoVector3D");
  _validation_state = NodeValidationState::Valid;

  addPort<DecimalData>(PortType::In, "X<Decimal>", true);
  addPort<DecimalData>(PortType::In, "Y<Decimal>", true);
  addPort<DecimalData>(PortType::In, "Z<Decimal>", true);

  addPort<Vector3DData>(PortType::Out, "Vector3D", true);
}

void XYZtoVector3DNode::compute()
{
  double x = static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())->value();
  double y = static_cast<DecimalData*>(_in_ports[1].in_value.lock().get())->value();
  double z = static_cast<DecimalData*>(_in_ports[2].in_value.lock().get())->value();

  _out_ports[0].out_value = std::make_shared<Vector3DData>(glm::vec3(x, y, z));
  _node->onDataUpdated(0);
}

NodeValidationState XYZtoVector3DNode::validate()
{

  if (!static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())
    || !static_cast<DecimalData*>(_in_ports[1].in_value.lock().get())
    || !static_cast<DecimalData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate decimal input.");
    return _validation_state;
  }

  return _validation_state;
}
