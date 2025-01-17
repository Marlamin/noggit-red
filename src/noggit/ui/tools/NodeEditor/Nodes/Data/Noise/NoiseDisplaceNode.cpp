// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseDisplaceNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseDisplaceNode::NoiseDisplaceNode()
: BaseNode()
{
  setName("Noise :: Displace");
  setCaption("Noise :: Displace");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<NoiseData>(PortType::In, "X<Noise>", true);
  addPortDefault<NoiseData>(PortType::In, "Y<Noise>", true);
  addPortDefault<NoiseData>(PortType::In, "Z<Noise>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseDisplaceNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());
  _module.SetXDisplaceModule(*static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())->value());
  _module.SetYDisplaceModule(*static_cast<NoiseData*>(_in_ports[2].in_value.lock().get())->value());
  _module.SetZDisplaceModule(*static_cast<NoiseData*>(_in_ports[3].in_value.lock().get())->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);

  _node->onDataUpdated(0);
}

NodeValidationState NoiseDisplaceNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())
    || !static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())
    || !static_cast<NoiseData*>(_in_ports[2].in_value.lock().get())
    || !static_cast<NoiseData*>(_in_ports[3].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
