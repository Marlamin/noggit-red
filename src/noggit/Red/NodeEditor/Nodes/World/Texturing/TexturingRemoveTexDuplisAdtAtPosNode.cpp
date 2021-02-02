// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TexturingRemoveTexDuplisAdtAtPosNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TexturingRemoveTexDuplisAdtAtPosNode::TexturingRemoveTexDuplisAdtAtPosNode()
: ContextLogicNodeBase()
{
  setName("Texturing :: RemoveTexDuplisADTAtPos");
  setCaption("Texturing :: RemoveTexDuplisADTAtPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TexturingRemoveTexDuplisAdtAtPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  world->removeTexDuplicateOnADT({pos.x, pos.y, pos.z});

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
