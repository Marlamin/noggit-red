// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MoveSelectedObjectInstances.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

MoveSelectedObjectInstancesNode::MoveSelectedObjectInstancesNode()
: ContextLogicNodeBase()
{
  setName("Selection :: MoveSelectedObjectInstances");
  setCaption("Selection :: MoveSelectedObjectInstances");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Delta<Vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void MoveSelectedObjectInstancesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto delta_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& delta = delta_data->value();

  world->move_selected_models({delta.x, delta.y, delta.z});

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
