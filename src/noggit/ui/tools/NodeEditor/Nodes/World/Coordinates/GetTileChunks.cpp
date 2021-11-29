// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetTileChunks.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

GetTileChunksNode::GetTileChunksNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: GetTileChunks");
  setCaption("Coordinates :: GetTileChunks");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);

  addPort<LogicData>(PortType::Out, "Logic", true);

  addPort<ListData>(PortType::Out, "List[Chunk]", true);
  _out_ports[1].data_type->set_parameter_type("chunk");
}

void GetTileChunksNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  _chunks.clear();
  _chunks.resize(256);
  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      MapChunk* chunk = tile->getChunk(i, j);
      _chunks[i * 16 + j] = std::make_shared<ChunkData>();
      NOGGIT_CUR_ACTION->registerAllChunkChanges(chunk);
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  auto list =  std::make_shared<ListData>(&_chunks);
  list->set_parameter_type("chunk");
  _out_ports[1].out_value = std::move(list);
  _node->onDataUpdated(1);

}

NodeValidationState GetTileChunksNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
