// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "HasTileAt.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

HasTileAtNode::HasTileAtNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: HasTileAt");
  setCaption("Coordinates :: HasTileAt");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector2DData>(PortType::In, "TileXY<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<BooleanData>(PortType::Out, "Boolean", true);
}

void HasTileAtNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec2 const& xy = defaultPortData<Vector3DData>(PortType::In, 1)->value();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<BooleanData>(world->mapIndex.hasTile(tile_index(xy.x, xy.y)));
  _node->onDataUpdated(1);
}
