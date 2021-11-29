// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetWaterType.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

GetWaterTypeNode::GetWaterTypeNode()
: ContextNodeBase()
{
  setName("Liquid :: GetLiquidTypeADTAtPos");
  setCaption("Liquid :: GetLiquidTypeADTAtPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3d>", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Layer<UInteger>", true);
  addPort<UnsignedIntegerData>(PortType::Out, "LiquidType<UInteger>", true);

}

void GetWaterTypeNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 0);
  glm::vec3 const& pos = pos_data->value();

  unsigned layer = defaultPortData<UnsignedIntegerData>(PortType::In, 1)->value();

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

  _out_ports[0].out_value = std::make_shared<UnsignedIntegerData>(world->getWaterType(n_pos, layer));
  _node->onDataUpdated(0);

}


