// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ObjectInstanceSetScale.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ObjectInstanceSetScaleNode::ObjectInstanceSetScaleNode()
: ContextLogicNodeBase()
{
  setName("Object :: SetScale");
  setCaption("Object :: SetScale");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ObjectInstanceData>(PortType::In, "ObjectInstance>", true);
  addPortDefault<DecimalData>(PortType::In, "Scale<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ObjectInstanceSetScaleNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  SceneObject* obj = defaultPortData<ObjectInstanceData>(PortType::In, 1)->value();
  float scale = defaultPortData<DecimalData>(PortType::In, 2)->value();

  if (scale < 0.0000001f)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: scale should be a non-zero positive value.");
    return;
  }

  obj->scale = scale;

  obj->recalcExtents();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState ObjectInstanceSetScaleNode::validate()
{
  if (!static_cast<ObjectInstanceData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate object instance input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

