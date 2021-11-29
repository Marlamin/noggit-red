// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ReloadTileNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

ReloadTileNode::ReloadTileNode()
: ContextLogicNodeBase()
{
  setName("Tile :: Reload");
  setCaption("Tile :: Reload");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector2DData>(PortType::In, "TileXY<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ReloadTileNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto xy_data = defaultPortData<Vector2DData>(PortType::In, 1);
  glm::vec2 const& xy = xy_data->value();


  if (!world->mapIndex.hasTile(tile_index(xy.x, xy.y)))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: tile index is out of range or tile does not exist.");
    return;
  }

  world->reload_tile(tile_index(xy.x, xy.y));

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
