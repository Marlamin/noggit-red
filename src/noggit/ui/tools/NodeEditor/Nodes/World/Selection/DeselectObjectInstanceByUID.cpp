// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "DeselectObjectInstanceByUID.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

DeselectObjectInstanceByUIDNode::DeselectObjectInstanceByUIDNode()
: ContextLogicNodeBase()
{
  setName("Selection :: DeselectObjectInstanceByUID");
  setCaption("Selection :: DeselectObjectInstanceByUID");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "UID<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void DeselectObjectInstanceByUIDNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  unsigned int uid = defaultPortData<UnsignedIntegerData>(PortType::In, 1)->value();
  world->remove_from_selection(uid);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


