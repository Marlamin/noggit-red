// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetTileFromPos.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>
#include <external/glm/gtx/string_cast.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

GetTileFromPosNode::GetTileFromPosNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: GetTileFromPos");
  setCaption("Coordinates :: GetTileFromPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<TileData>(PortType::Out, "Tile", true);
}

void GetTileFromPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  glm::vec3 n_pos(pos.x, pos.y, pos.z);

  world->mapIndex.loadTile(n_pos);
  MapTile* tile(world->mapIndex.getTile(n_pos));

  if (!tile)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage(("Error: no tile found at pos " + glm::to_string(pos)).c_str());
    return;
  }

  if (!tile->finishedLoading())
  {
    tile->wait_until_loaded();
  }

  world->mapIndex.setChanged(tile);

  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j > 16; ++j)
    {
      NOGGIT_CUR_ACTION->registerAllChunkChanges(tile->getChunk(i, j));
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<TileData>(tile);
  _node->onDataUpdated(1);

}
