// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseCheckerboardNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

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
  auto module = new noise::module::Checkerboard();

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  _node->onDataUpdated(0);
}
