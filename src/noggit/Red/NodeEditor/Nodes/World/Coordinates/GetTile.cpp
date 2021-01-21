// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetTile.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
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

  glm::vec2 const& xy = defaultPortData<Vector3DData>(PortType::In, 1)->value();

  MapTile* tile = world->mapIndex.getTile(tile_index(xy.x, xy.y));

  if (!tile)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: tile coordinates are out of range or tile does not exist.");
    return;
  }

  if (!tile->finishedLoading())
  {
    world->mapIndex.loadTile(tile_index(xy.x, xy.y));
    tile->wait_until_loaded();
  }

  world->mapIndex.setChanged(tile);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<TileData>(tile);
  Q_EMIT dataUpdated(1);


}
