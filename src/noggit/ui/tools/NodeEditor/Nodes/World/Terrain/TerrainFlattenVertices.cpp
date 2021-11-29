// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TerrainFlattenVertices.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TerrainFlattenVerticesNode::TerrainFlattenVerticesNode()
: ContextLogicNodeBase()
{
  setName("Terrain :: FlattenVertices");
  setCaption("Terrain :: FlattenVertices");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<DecimalData>(PortType::In, "Height<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TerrainFlattenVerticesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  world->flattenVertices(defaultPortData<DecimalData>(PortType::In, 1)->value());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

