// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ResetSelection.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ResetSelectionNode::ResetSelectionNode()
: ContextLogicNodeBase()
{
  setName("Selection :: ResetSelectionNode");
  setCaption("Selection :: ResetSelection");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ResetSelectionNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  world->reset_selection();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

