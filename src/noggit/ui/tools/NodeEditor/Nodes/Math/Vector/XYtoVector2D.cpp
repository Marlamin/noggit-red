// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "XYtoVector2D.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

XYtoVector2DNode::XYtoVector2DNode()
: BaseNode()
{
  setName("Vector :: XYtoVector2D");
  setCaption("Vector :: XYtoVector2D");
  _validation_state = NodeValidationState::Valid;

  addPort<DecimalData>(PortType::In, "X<Decimal>", true);
  addPort<DecimalData>(PortType::In, "Y<Decimal>", true);

  addPort<Vector2DData>(PortType::Out, "Vector2D", true);
}

void XYtoVector2DNode::compute()
{
  double x = static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())->value();
  double y = static_cast<DecimalData*>(_in_ports[1].in_value.lock().get())->value();

  _out_ports[0].out_value = std::make_shared<Vector2DData>(glm::vec2(x, y));
  _node->onDataUpdated(0);
}

NodeValidationState XYtoVector2DNode::validate()
{

  if (!static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())
    || !static_cast<DecimalData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate decimal input.");
    return _validation_state;
  }

  return _validation_state;
}
