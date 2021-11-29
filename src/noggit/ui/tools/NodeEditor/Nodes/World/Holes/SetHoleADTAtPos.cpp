// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SetHoleADTAtPos.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

SetHoleADTAtPosNode::SetHoleADTAtPosNode()
: ContextLogicNodeBase()
{
  setName("Holes :: SetHoleADTAtPosNode");
  setCaption("Holes :: SetHoleADTAtPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<BooleanData>(PortType::In, "Add<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SetHoleADTAtPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  bool add = defaultPortData<BooleanData>(PortType::In, 2)->value();

  world->setHoleADT({pos.x, pos.y, pos.z}, add);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

