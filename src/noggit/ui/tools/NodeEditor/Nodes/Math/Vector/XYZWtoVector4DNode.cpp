// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "XYZWtoVector4DNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

XYZWtoVector4DNode::XYZWtoVector4DNode()
: BaseNode()
{
  setName("Vector :: XYZWtoVector4D");
  setCaption("Vector :: XYZWToVector4D");
  _validation_state = NodeValidationState::Valid;

  addPort<DecimalData>(PortType::In, "X<Decimal>", true);
  addPort<DecimalData>(PortType::In, "Y<Decimal>", true);
  addPort<DecimalData>(PortType::In, "Z<Decimal>", true);
  addPort<DecimalData>(PortType::In, "W<Decimal>", true);

  addPort<Vector4DData>(PortType::Out, "Vector4D", true);
}

void XYZWtoVector4DNode::compute()
{
  double x = static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())->value();
  double y = static_cast<DecimalData*>(_in_ports[1].in_value.lock().get())->value();
  double z = static_cast<DecimalData*>(_in_ports[2].in_value.lock().get())->value();
  double w = static_cast<DecimalData*>(_in_ports[3].in_value.lock().get())->value();

  _out_ports[0].out_value = std::make_shared<Vector4DData>(glm::vec4(x, y, z, w));
  _node->onDataUpdated(0);
}

NodeValidationState XYZWtoVector4DNode::validate()
{

  if (!static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())
    || !static_cast<DecimalData*>(_in_ports[1].in_value.lock().get())
    || !static_cast<DecimalData*>(_in_ports[2].in_value.lock().get())
    || !static_cast<DecimalData*>(_in_ports[3].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate decimal input.");
    return _validation_state;
  }

  return _validation_state;
}
