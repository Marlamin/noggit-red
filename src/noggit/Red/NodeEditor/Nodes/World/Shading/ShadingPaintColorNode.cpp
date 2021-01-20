// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ShadingPaintColorNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ShadingPaintColorNode::ShadingPaintColorNode()
: ContextLogicNodeBase()
{
  setName("ShadingPaintColorNode");
  setCaption("Shading Paint Color");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);

  auto radius = static_cast<QDoubleSpinBox*>(_in_ports[2].default_widget);
  radius->setMinimum(0);

  addPortDefault<DecimalData>(PortType::In, "Change<Decimal>", true);
  addPortDefault<ColorData>(PortType::In, "Color<Decimal>", true);
  addPortDefault<BooleanData>(PortType::In, "Subtract<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ShadingPaintColorNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec3 const& pos = defaultPortData<Vector3DData>(PortType::In, 1)->value();
  double radius = std::max(0.0, defaultPortData<DecimalData>(PortType::In, 2)->value());
  double change = defaultPortData<DecimalData>(PortType::In, 3)->value();
  glm::vec4 const& color = defaultPortData<ColorData>(PortType::In, 4)->value();
  bool subtract = defaultPortData<BooleanData>(PortType::In, 5)->value();

  world->changeShader({pos.x, pos.y, pos.z}, {color.r, color.g, color.b, color.a}, change, radius, subtract);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

}
