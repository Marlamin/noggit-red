// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetChunk.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

GetChunkNode::GetChunkNode()
: ContextLogicNodeBase()
{
  setName("GetChunkNode");
  setCaption("Get Chunk");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);
  addPortDefault<Vector2DData>(PortType::In, "ChunkXY<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ChunkData>(PortType::Out, "Chunk", true);
}

void GetChunkNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  glm::vec2 const& xy = defaultPortData<Vector2DData>(PortType::In, 2)->value();

  MapChunk* chunk;
  if (xy.x < 0 || xy.x > 15.4f || xy.y < 0 || xy.y > 15.4f || !(chunk = tile->getChunk(xy.x, xy.y)))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: chunk coordinates are out of range.");
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ChunkData>(chunk);
  Q_EMIT dataUpdated(1);

}
