// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetTile.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>
#include <external/glm/gtx/string_cast.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

GetTileNode::GetTileNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: GetTile");
  setCaption("Coordinates :: GetTile");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector2DData>(PortType::In, "TileXY<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<TileData>(PortType::Out, "Tile", true);
}

void GetTileNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto xy_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec2 const& xy = xy_data->value();

  world->mapIndex.loadTile(tile_index(xy.x, xy.y));
  MapTile* tile = world->mapIndex.getTile(tile_index(xy.x, xy.y));

  if (!tile)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: tile coordinates are out of range or tile does not exist.");
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
      noggit::ActionManager::instance()->getCurrentAction()->registerAllChunkChanges(tile->getChunk(i, j));
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<TileData>(tile);
  _node->onDataUpdated(1);


}
