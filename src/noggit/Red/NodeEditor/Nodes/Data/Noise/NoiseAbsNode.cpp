// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseAbsNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseAbsNode::NoiseAbsNode()
: BaseNode()
{
  setName("NoiseAbsNode");
  setCaption("Noise Abs");
  _validation_state = NodeValidationState::Valid;

  addPort<NoiseData>(PortType::In, "Noise", true);
  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseAbsNode::compute()
{
  auto module = new noise::module::Abs();
  module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);

}

NodeValidationState NoiseAbsNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}

