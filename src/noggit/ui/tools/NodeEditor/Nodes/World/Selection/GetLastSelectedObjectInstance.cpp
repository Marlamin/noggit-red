// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetLastSelectedObjectInstance.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>
#include <noggit/World.h>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

GetLastSelectedObjectInstanceNode::GetLastSelectedObjectInstanceNode()
: ContextLogicNodeBase()
{
  setName("Selection :: GetLastSelectedObjectInstanceNode");
  setCaption("Selection :: GetLastSelectedObjectInstance");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ObjectInstanceData>(PortType::Out, "ObjectInstance", true);
}

void GetLastSelectedObjectInstanceNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto obj_optional = world->get_last_selected_model();

  if (!obj_optional)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: nothing was selected.");
    return;
  }

  SceneObject* obj = std::get<selected_object_type>(obj_optional.value());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ObjectInstanceData>(obj);
  _node->onDataUpdated(1);

}

