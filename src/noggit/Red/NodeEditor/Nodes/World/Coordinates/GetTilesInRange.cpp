// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetTilesInRange.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

GetTilesInRangeNode::GetTilesInRangeNode()
: ContextLogicNodeBase()
{
  setName("Coordinates :: GetTilesInRange");
  setCaption("Coordinates :: GetTilesInRange");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);

  auto radius = static_cast<QDoubleSpinBox*>(_in_ports[2].default_widget);
  radius->setMinimum(0);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ListData>(PortType::Out, "List[Tile]", true);
  _out_ports[1].data_type->set_parameter_type("tile");
}

void GetTilesInRangeNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec3 const& pos = defaultPortData<Vector3DData>(PortType::In, 1)->value();
  double radius = std::max(0.0, defaultPortData<DecimalData>(PortType::In, 2)->value());

  for (MapTile* tile : world->mapIndex.tiles_in_range({pos.x, pos.y, pos.z}, radius))
  {
    if (!tile->finishedLoading())
    {
      tile->wait_until_loaded();
    }

    _tiles.push_back(std::make_shared<TileData>(tile));
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  auto list =  std::make_shared<ListData>(&_tiles);
  list->set_parameter_type("tile");
  _out_ports[1].out_value = std::move(list);
  Q_EMIT dataUpdated(1);

}
