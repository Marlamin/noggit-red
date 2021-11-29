// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ColorToRGBANode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

ColorToRGBANode::ColorToRGBANode()
: BaseNode()
{
  setName("Color :: ColorToRGBA");
  setCaption("Color :: ColorToRGBA");
  _validation_state = NodeValidationState::Valid;

  addPort<ColorData>(PortType::In, "Color", true);
  addPort<DecimalData>(PortType::Out, "R<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "G<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "B<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "A<Decimal>", true);
}

void ColorToRGBANode::compute()
{
  glm::vec4 color = static_cast<ColorData*>(_in_ports[0].in_value.lock().get())->value();

  _out_ports[0].out_value = std::make_shared<DecimalData>(color.r);
  _out_ports[1].out_value = std::make_shared<DecimalData>(color.g);
  _out_ports[2].out_value = std::make_shared<DecimalData>(color.b);
  _out_ports[3].out_value = std::make_shared<DecimalData>(color.a);

  _node->onDataUpdated(0);
  _node->onDataUpdated(1);
  _node->onDataUpdated(2);
  _node->onDataUpdated(3);
}

NodeValidationState ColorToRGBANode::validate()
{
  if (!static_cast<ColorData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate color input.");
    return _validation_state;
  }

  return _validation_state;
}
