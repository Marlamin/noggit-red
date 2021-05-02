// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ObjectInstanceInfo.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ObjectInstanceInfoNode::ObjectInstanceInfoNode()
: ContextNodeBase()
{
  setName("Object :: InstanceInfo");
  setCaption("Object :: Instance");
  _validation_state = NodeValidationState::Valid;

  addPort<ObjectInstanceData>(PortType::In, "ObjectInstance", true);

  addPort<Vector3DData>(PortType::Out, "Pos<Vector3D>", true);
  addPort<Vector3DData>(PortType::Out, "Rotation<Vector3D>", true);
  addPort<DecimalData>(PortType::Out, "Scale<Decimal>", true);
  addPort<Vector3DData>(PortType::Out, "BoundsLower<Vector3D>", true);
  addPort<Vector3DData>(PortType::Out, "BoundsUpper<Vector3D>", true);
  addPort<Vector3DData>(PortType::Out, "BoundsUpper<Vector3D>", true);
  addPort<UnsignedIntegerData>(PortType::Out, "UID<UInteger>", true);
  addPort<StringData>(PortType::Out, "Filename<String>", true);
}

void ObjectInstanceInfoNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  SceneObject* obj = defaultPortData<ObjectInstanceData>(PortType::In, 0)->value();

  if (_out_ports[0].connected)
  {
    _out_ports[0].out_value = std::make_shared<Vector3DData>(glm::vec3(obj->pos.x, obj->pos.y, obj->pos.z));
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    _out_ports[1].out_value = std::make_shared<Vector3DData>(glm::vec3(obj->dir.x._, obj->dir.y._, obj->dir.z._));
    _node->onDataUpdated(1);
  }

  if (_out_ports[2].connected)
  {
    _out_ports[2].out_value = std::make_shared<DecimalData>(obj->which() == eMODEL ? obj->scale : 1.0);
    _node->onDataUpdated(2);
  }

  if (_out_ports[3].connected)
  {
    _out_ports[3].out_value = std::make_shared<Vector3DData>(glm::vec3(obj->extents[0].x,
                                                                       obj->extents[0].y,
                                                                       obj->extents[0].z));
    _node->onDataUpdated(3);
  }

  if (_out_ports[4].connected)
  {
    _out_ports[4].out_value = std::make_shared<Vector3DData>(glm::vec3(obj->extents[1].x,
                                                                       obj->extents[1].y,
                                                                       obj->extents[1].z));
    _node->onDataUpdated(4);
  }

  if (_out_ports[5].connected)
  {
    _out_ports[5].out_value = std::make_shared<UnsignedIntegerData>(obj->which() == eMODEL ?
        static_cast<ModelInstance*>(obj)->uid
        : static_cast<WMOInstance*>(obj)->uid);

    _node->onDataUpdated(5);
  }

  if (_out_ports[6].connected)
  {
    _out_ports[6].out_value = std::make_shared<StringData>(obj->getFilename());

    _node->onDataUpdated(6);
  }

}

NodeValidationState ObjectInstanceInfoNode::validate()
{
  if (!static_cast<ObjectInstanceData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate object instance input.");
    return _validation_state;
  }

  return ContextNodeBase::validate();
}
