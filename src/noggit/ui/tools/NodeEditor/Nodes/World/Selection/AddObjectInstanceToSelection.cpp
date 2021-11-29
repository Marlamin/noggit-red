// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "AddObjectInstanceToSelection.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

AddObjectInstanceToSelectionNode::AddObjectInstanceToSelectionNode()
: ContextLogicNodeBase()
{
  setName("Selection :: AddObjectInstanceToSelection");
  setCaption("Selection :: AddObjectInstanceToSelection");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ObjectInstanceData>(PortType::In, "ObjectInstance", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void AddObjectInstanceToSelectionNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  SceneObject* obj = defaultPortData<ObjectInstanceData>(PortType::In, 1)->value();

  world->add_to_selection(obj);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState AddObjectInstanceToSelectionNode::validate()
{

  if (!static_cast<ObjectInstanceData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate object instance input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

