// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TerrainFlattenNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

TerrainFlattenNode::TerrainFlattenNode()
: ContextLogicNodeBase()
{
  setName("Terrain :: Flatten");
  setCaption("Terrain :: Flatten");
  _validation_state = NodeValidationState::Valid;

  _mode = new QComboBox(&_embedded_widget);
  _mode->addItems({"Flat",
                   "Linear",
                   "Smooth"});
  addWidgetTop(_mode);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Remain<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);
  addPortDefault<BooleanData>(PortType::In, "Raise<Decimal>", true);
  addPortDefault<BooleanData>(PortType::In, "Lower<Decimal>", true);
  addPortDefault<Vector3DData>(PortType::In, "Origin<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Angle<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "Orientation<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TerrainFlattenNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec3 pos = defaultPortData<Vector3DData>(PortType::In, 1)->value();
  glm::vec3 origin = defaultPortData<Vector3DData>(PortType::In, 6)->value();

  world->flattenTerrain({pos.x, pos.y, pos.z},
                       defaultPortData<DecimalData>(PortType::In, 2)->value(),
                       defaultPortData<DecimalData>(PortType::In, 3)->value(),
                       _mode->currentIndex(),
                       {defaultPortData<BooleanData>(PortType::In, 4)->value(),
                        defaultPortData<BooleanData>(PortType::In, 5)->value()},
                        {origin.x, origin.y, origin.z},
                        math::degrees(defaultPortData<DecimalData>(PortType::In, 7)->value()),
                        math::degrees(defaultPortData<DecimalData>(PortType::In, 8)->value()));

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


QJsonObject TerrainFlattenNode::save() const
{
  QJsonObject json_obj = ContextLogicNodeBase::save();

  json_obj["mode"] = _mode->currentIndex();

  return json_obj;
}

void TerrainFlattenNode::restore(const QJsonObject& json_obj)
{
  ContextLogicNodeBase::restore(json_obj);
  _mode->setCurrentIndex(json_obj["mode"].toInt());
}
