// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TerrainRaiseLowerNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TerrainRaiseLowerNode::TerrainRaiseLowerNode()
: ContextLogicNodeBase()
{
  setName("TerrainRaiseLowerNode");
  setCaption("Terrain Raise/Lower");
  _validation_state = NodeValidationState::Valid;

  _mode = new QComboBox(&_embedded_widget);
  _mode->addItems({"Flat",
                         "Linear",
                         "Smooth",
                         "Polynomial",
                         "Trigonom",
                         "Quadratic",
                         "Gaussian"});
  addWidgetTop(_mode);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Diff<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "InnerRadius<Decimal>", true);
  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TerrainRaiseLowerNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec3 pos = defaultPortData<Vector3DData>(PortType::In, 1)->value();

  world->changeTerrain({pos.x, pos.y, pos.z},
                       defaultPortData<DecimalData>(PortType::In, 2)->value(),
                       defaultPortData<DecimalData>(PortType::In, 3)->value(),
                       _mode->currentIndex(),
                       defaultPortData<DecimalData>(PortType::In, 4)->value());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

}


QJsonObject TerrainRaiseLowerNode::save() const
{
  QJsonObject json_obj = ContextLogicNodeBase::save();

  json_obj["mode"] = _mode->currentIndex();

  return json_obj;
}

void TerrainRaiseLowerNode::restore(const QJsonObject& json_obj)
{
  ContextLogicNodeBase::restore(json_obj);
  _mode->setCurrentIndex(json_obj["mode"].toInt());
}

