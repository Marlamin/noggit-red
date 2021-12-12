// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetObjectInstanceByUID.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

GetObjectInstanceByUIDNode::GetObjectInstanceByUIDNode()
: ContextLogicNodeBase()
{
  setName("Object:: GetObjectInstanceByUID");
  setCaption("Object :: GetObjectInstanceByUID");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "UID<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ObjectInstanceData>(PortType::Out, "ObjectInstance", true);
}

void GetObjectInstanceByUIDNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  unsigned int uid = defaultPortData<UnsignedIntegerData>(PortType::In, 1)->value();

  auto obj_optional = world->get_model(uid);

  if (!obj_optional)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage(("Error: object with provided UID +" + std::to_string(uid) + "was not found.").c_str());
    return;
  }

  SceneObject* obj = boost::get<selected_object_type>(obj_optional.get());

  NOGGIT_CUR_ACTION->registerObjectTransformed(obj);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ObjectInstanceData>(obj);
  _node->onDataUpdated(1);

}
