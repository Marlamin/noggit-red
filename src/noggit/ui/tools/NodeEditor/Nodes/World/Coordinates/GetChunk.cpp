// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetChunk.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

GetChunkNode::GetChunkNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: GetChunk");
  setCaption("Coordinates :: GetChunk");
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
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  auto xy_data = defaultPortData<Vector2DData>(PortType::In, 2);
  glm::vec2 const& xy = xy_data->value();

  MapChunk* chunk;
  if (xy.x < 0 || xy.x > 15.4f || xy.y < 0 || xy.y > 15.4f || !(chunk = tile->getChunk(xy.x, xy.y)))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: chunk coordinates are out of range.");
    return;
  }

  NOGGIT_CUR_ACTION->registerAllChunkChanges(chunk);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ChunkData>(chunk);
  _node->onDataUpdated(1);

}

NodeValidationState GetChunkNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
