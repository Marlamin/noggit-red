// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RGBAtoColorNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

RGBAtoColorNode::RGBAtoColorNode()
: BaseNode()
{
  setName("Color :: RGBAtoColor");
  setCaption("Color :: RGBAToColor");
  _validation_state = NodeValidationState::Valid;
}

void RGBAtoColorNode::compute()
{
  double r = static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())->value();
  double g = static_cast<DecimalData*>(_in_ports[1].in_value.lock().get())->value();
  double b = static_cast<DecimalData*>(_in_ports[2].in_value.lock().get())->value();
  double a = static_cast<DecimalData*>(_in_ports[3].in_value.lock().get())->value();

  _out_ports[0].out_value = std::make_shared<ColorData>(glm::vec4(r, g, b, a));
  _node->onDataUpdated(0);
}

NodeValidationState RGBAtoColorNode::validate()
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
