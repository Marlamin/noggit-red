// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ObjectInstanceSetRotation.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

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
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  SceneObject* obj = defaultPortData<ObjectInstanceData>(PortType::In, 1)->value();
  NOGGIT_CUR_ACTION->registerObjectTransformed(obj);
  auto rot_data = defaultPortData<Vector3DData>(PortType::In, 2);
  glm::vec3 const& rotation = rot_data->value();

  obj->dir.x = math::degrees(rotation.x)._;
  obj->dir.y = math::degrees(rotation.y)._;
  obj->dir.z = math::degrees(rotation.z)._;

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
