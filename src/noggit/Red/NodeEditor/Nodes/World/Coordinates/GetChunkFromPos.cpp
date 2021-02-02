// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetChunkFromPos.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>
#include <external/glm/gtx/string_cast.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

GetChunkFromPosNode::GetChunkFromPosNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: GetChunkFromPos");
  setCaption("Coordinates :: GetChunkFromPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ChunkData>(PortType::Out, "Chunk", true);
}

void GetChunkFromPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  math::vector_3d n_pos(pos.x, pos.y, pos.z);

  MapTile* tile(world->mapIndex.getTile(n_pos));

  if (!tile)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage(("Error: no tile found at pos " + glm::to_string(pos)).c_str());
    return;
  }

  if (!tile->finishedLoading())
  {
    world->mapIndex.loadTile(n_pos);
    tile->wait_until_loaded();
  }

  world->mapIndex.setChanged(tile);


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ChunkData>(tile->getChunk((pos.x - tile->xbase) / CHUNKSIZE,
                                                                                  (pos.z - tile->zbase) / CHUNKSIZE));
  _node->onDataUpdated(1);

}
