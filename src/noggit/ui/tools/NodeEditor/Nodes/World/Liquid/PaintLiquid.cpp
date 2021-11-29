// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "PaintLiquid.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/DBC.h>

using namespace noggit::Red::NodeEditor::Nodes;

PaintLiquidNode::PaintLiquidNode()
: ContextLogicNodeBase()
{
  setName("Liquid :: PaintLiquid");
  setCaption("Liquid :: PaintLiquid");
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
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);
  addPortDefault<BooleanData>(PortType::In, "Add<Boolean>", true);
  addPortDefault<DecimalData>(PortType::In, "Angle<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "Orientation<Decimal>", true);
  addPortDefault<BooleanData>(PortType::In, "Lock<Boolean>", true);
  addPortDefault<Vector3DData>(PortType::In, "Origin<Vector3D>", true);
  addPortDefault<BooleanData>(PortType::In, "OverrideHeight<Boolean>", true);
  addPortDefault<BooleanData>(PortType::In, "OverrideLiquidType<Boolean>", true);
  addPortDefault<DecimalData>(PortType::In, "OpacityFactor<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void PaintLiquidNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  double radius = glm::radians(defaultPortData<DecimalData>(PortType::In, 2)->value());

  bool add = defaultPortData<BooleanData>(PortType::In, 3)->value();
  double angle = glm::radians(defaultPortData<DecimalData>(PortType::In, 4)->value());
  double orientation = glm::radians(defaultPortData<DecimalData>(PortType::In, 5)->value());
  bool lock = defaultPortData<BooleanData>(PortType::In, 6)->value();

  auto origin_data = defaultPortData<Vector3DData>(PortType::In, 7);
  glm::vec3 const& origin = origin_data->value();

  bool override_height = defaultPortData<BooleanData>(PortType::In, 8)->value();
  bool override_liquid_id = defaultPortData<BooleanData>(PortType::In, 9)->value();
  double opacity_factor = defaultPortData<DecimalData>(PortType::In, 10)->value();

  world->paintLiquid({pos.x, pos.y, pos.z},
                     radius,
                     _liquid_type->currentData().toInt(),
                     add,
                     math::radians(angle),
                     math::radians(orientation),
                     lock,
                     {origin.x, origin.y, origin.z},
                     override_height,
                     override_liquid_id,
                     opacity_factor);


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


QJsonObject PaintLiquidNode::save() const
{
  QJsonObject json_obj = ContextLogicNodeBase::save();

  json_obj["liquid_type"] = _liquid_type->currentIndex();

  return json_obj;
}

void PaintLiquidNode::restore(const QJsonObject& json_obj)
{
  ContextLogicNodeBase::restore(json_obj);

  _liquid_type->setCurrentIndex(json_obj["liquid_type"].toInt());
}


