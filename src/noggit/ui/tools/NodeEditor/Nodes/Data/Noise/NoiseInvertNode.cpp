// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseInvertNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseInvertNode::NoiseInvertNode()
: BaseNode()
{
  setName("Noise :: Invert");
  setCaption("Noise :: Invert");
  _validation_state = NodeValidationState::Valid;

  addPort<NoiseData>(PortType::In, "Noise", true);
  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseInvertNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);
}

NodeValidationState NoiseInvertNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
