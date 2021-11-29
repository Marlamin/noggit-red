// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseCheckerboardNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseCheckerboardNode::NoiseCheckerboardNode()
: NoiseGeneratorBase()
{
  setName("Noise :: Checkerboard");
  setCaption("Noise :: Checkerboard");
  _validation_state = NodeValidationState::Valid;

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseCheckerboardNode::compute()
{
  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);
}
