// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ObjectInstanceSetRotation.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ObjectInstanceSetRotationNode::ObjectInstanceSetRotationNode()
: ContextLogicNodeBase()
{
  setName("Object :: SetRotation");
  setCaption("Object :: SetRotation");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ObjectInstanceData>(PortType::In, "ObjectInstance", true);
  addPortDefault<Vector3DData>(PortType::In, "Rotation<Vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ObjectInstanceSetRotationNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  SceneObject* obj = defaultPortData<ObjectInstanceData>(PortType::In, 1)->value();
  auto rot_data = defaultPortData<Vector3DData>(PortType::In, 2);
  glm::vec3 const& rotation = rot_data->value();

  obj->dir.x = math::degrees(rotation.x);
  obj->dir.y = math::degrees(rotation.y);
  obj->dir.z = math::degrees(rotation.z);

  obj->recalcExtents();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
}


NodeValidationState ObjectInstanceSetRotationNode::validate()
{
  if (!static_cast<ObjectInstanceData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate object instance input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
