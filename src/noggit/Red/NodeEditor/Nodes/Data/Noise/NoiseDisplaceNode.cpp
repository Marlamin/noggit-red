// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseDisplaceNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseDisplaceNode::NoiseDisplaceNode()
: BaseNode()
{
  setName("NoiseDisplaceNode");
  setCaption("Noise Displace");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<NoiseData>(PortType::In, "X<Noise>", true);
  addPortDefault<NoiseData>(PortType::In, "Y<Noise>", true);
  addPortDefault<NoiseData>(PortType::In, "Z<Noise>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseDisplaceNode::compute()
{
  auto module = new noise::module::Displace();
  module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());
  module->SetXDisplaceModule(*static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())->value().get());
  module->SetYDisplaceModule(*static_cast<NoiseData*>(_in_ports[2].in_value.lock().get())->value().get());
  module->SetZDisplaceModule(*static_cast<NoiseData*>(_in_ports[3].in_value.lock().get())->value().get());

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);
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
