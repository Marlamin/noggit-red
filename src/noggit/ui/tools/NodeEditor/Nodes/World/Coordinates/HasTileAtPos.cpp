// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "HasTileAtPos.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

HasTileAtPosNode::HasTileAtPosNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: HasTileAtPos");
  setCaption("Coordinates :: HasTileAtPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<BooleanData>(PortType::Out, "Boolean", true);
}

void HasTileAtPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<BooleanData>(world->mapIndex.hasTile(glm::vec3(pos.x, pos.y, pos.z)));
  _node->onDataUpdated(1);

}
