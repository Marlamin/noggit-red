// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TerrainDeselectVertices.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TerrainDeselectVerticesNode::TerrainDeselectVerticesNode()
: ContextLogicNodeBase()
{
  setName("Terrain :: DeselectVertices");
  setCaption("Terrain :: DeselectVertices");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);


  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TerrainDeselectVerticesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  world->deselectVertices({pos.x, pos.y, pos.z}, defaultPortData<DecimalData>(PortType::In, 2)->value());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


