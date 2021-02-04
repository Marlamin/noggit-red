// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RotateSelectedObjectInstances.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

RotateSelectedObjectInstancesNode::RotateSelectedObjectInstancesNode()
: ContextLogicNodeBase()
{
  setName("Selection :: RotateSelectedObjectInstances");
  setCaption("Selection :: RotateSelectedObjectInstances");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Rotation<Vector3D>", true);
  addPortDefault<BooleanData>(PortType::In, "UsePivot<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void RotateSelectedObjectInstancesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto rot_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& rot = rot_data->value();

  bool use_pivot = defaultPortData<BooleanData>(PortType::In, 2)->value();

  world->rotate_selected_models(math::degrees(rot.x), math::degrees(rot.y), math::degrees(rot.z), use_pivot);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

