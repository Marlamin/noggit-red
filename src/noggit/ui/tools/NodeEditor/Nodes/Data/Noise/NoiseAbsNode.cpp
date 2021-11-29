// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseAbsNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseAbsNode::NoiseAbsNode()
: BaseNode()
{
  setName("Noise :: Abs");
  setCaption("Noise :: Abs");
  _validation_state = NodeValidationState::Valid;

  addPort<NoiseData>(PortType::In, "Noise", true);
  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseAbsNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);

  _node->onDataUpdated(0);

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

