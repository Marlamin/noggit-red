// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SetWaterType.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/DBC.h>

using namespace noggit::Red::NodeEditor::Nodes;

SetWaterTypeNode::SetWaterTypeNode()
: ContextLogicNodeBase()
{
  setName("Liquid :: SetLiquidTypeADTAtPos");
  setCaption("Liquid :: SetLiquidTypeADTAtPos");
  _validation_state = NodeValidationState::Valid;

  _liquid_type = new QComboBox(&_embedded_widget);

  for (DBCFile::Iterator i = gLiquidTypeDB.begin(); i != gLiquidTypeDB.end(); ++i)
  {
    int liquid_id = i->getInt(LiquidTypeDB::ID);

    std::stringstream ss;
    ss << liquid_id << "-" << LiquidTypeDB::getLiquidName(liquid_id);
    _liquid_type->addItem (QString::fromUtf8(ss.str().c_str()), QVariant (liquid_id));
  }

  addWidgetTop(_liquid_type);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Layer<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SetWaterTypeNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  unsigned layer = defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value();

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

  world->setWaterType(n_pos, _liquid_type->currentData().toInt(), layer);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


QJsonObject SetWaterTypeNode::save() const
{
  QJsonObject json_obj = ContextLogicNodeBase::save();

  json_obj["liquid_type"] = _liquid_type->currentIndex();

  return json_obj;
}

void SetWaterTypeNode::restore(const QJsonObject& json_obj)
{
  ContextLogicNodeBase::restore(json_obj);

  _liquid_type->setCurrentIndex(json_obj["liquid_type"].toInt());
}


