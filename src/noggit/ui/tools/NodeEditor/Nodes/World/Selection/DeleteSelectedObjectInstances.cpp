// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "DeleteSelectedObjectInstances.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

DeleteSelectedObjectInstancesNode::DeleteSelectedObjectInstancesNode()
: ContextLogicNodeBase()
{
  setName("Selection :: DeleteSelectedObjectInstances");
  setCaption("Selection :: DeleteSelectedObjectInstances");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true);
}

void DeleteSelectedObjectInstancesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  world->delete_selected_models();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

