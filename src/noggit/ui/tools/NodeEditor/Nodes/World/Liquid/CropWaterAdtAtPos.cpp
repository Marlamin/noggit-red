// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "CropWaterAdtAtPos.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

CropWaterADTAtPosNode::CropWaterADTAtPosNode()
: ContextLogicNodeBase()
{
  setName("Liquid :: CropLiquidADTAtPos");
  setCaption("Liquid :: CropLiquidADTAtPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void CropWaterADTAtPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

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

  world->CropWaterADT(n_pos);


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

