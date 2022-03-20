// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetChunkFromPos.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>
#include <external/glm/gtx/string_cast.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

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
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  glm::vec3 n_pos(pos.x, pos.y, pos.z);

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

  MapChunk* chunk = tile->getChunk((pos.x - tile->xbase) / CHUNKSIZE,
                                   (pos.z - tile->zbase) / CHUNKSIZE);
  NOGGIT_CUR_ACTION->registerAllChunkChanges(chunk);

  _out_ports[1].out_value = std::make_shared<ChunkData>(chunk);
  _node->onDataUpdated(1);

}
