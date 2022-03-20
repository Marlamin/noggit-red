// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "IsObjectInstanceSelectedUID.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

IsObjectInstanceSelectedUIDNode::IsObjectInstanceSelectedUIDNode()
: ContextLogicNodeBase()
{
  setName("Selection :: IsObjectInstanceSelectedUID");
  setCaption("Selection ::  IsObjectInstanceSelectedUID");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "UID<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<BooleanData>(PortType::Out, "Boolean", true);
}

void IsObjectInstanceSelectedUIDNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  unsigned int uid = defaultPortData<UnsignedIntegerData>(PortType::In, 1)->value();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<BooleanData>(world->is_selected(uid));
  _node->onDataUpdated(1);
}

