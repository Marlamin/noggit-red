// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SnapSelectedObjectInstancesToGround.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

SnapSelectedObjectInstancesToGroundNode::SnapSelectedObjectInstancesToGroundNode()
: ContextLogicNodeBase()
{
  setName("Selection :: SnapSelectedObjectInstancesToGround");
  setCaption("Selection :: SnapSelectedObjectInstancesToGround");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SnapSelectedObjectInstancesToGroundNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  world->snap_selected_models_to_the_ground();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
