// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "FixAllGapsNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

FixAllGapsNode::FixAllGapsNode()
: ContextLogicNodeBase()
{
  setName("Loaded Tiles :: FixAllGaps");
  setCaption("Loaded Tiles :: FixAllGaps");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true);
}

void FixAllGapsNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  world->fixAllGaps();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
