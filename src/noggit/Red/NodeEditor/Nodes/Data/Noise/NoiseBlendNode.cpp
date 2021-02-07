// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseBlendNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseBlendNode::NoiseBlendNode()
: BaseNode()
{
  setName("Noise :: Blend");
  setCaption("Noise :: Blend");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<NoiseData>(PortType::In, "Factor<Noise>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseBlendNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());
  _module.SetSourceModule(1, *static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())->value());
  _module.SetControlModule(*static_cast<NoiseData*>(_in_ports[2].in_value.lock().get())->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);

  _node->onDataUpdated(0);

}

NodeValidationState NoiseBlendNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())
  || !static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())
  || !static_cast<NoiseData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
