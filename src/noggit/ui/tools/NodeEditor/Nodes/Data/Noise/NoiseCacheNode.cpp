// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseCacheNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseCacheNode::NoiseCacheNode()
: BaseNode()
{
  setName("Noise :: Cache");
  setCaption("Noise :: Cache");
  _validation_state = NodeValidationState::Valid;

  addPort<NoiseData>(PortType::In, "Noise", true);
  addPort<NoiseData>(PortType::Out, "Noise", true);

}

void NoiseCacheNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);

  _node->onDataUpdated(0);
}

NodeValidationState NoiseCacheNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
