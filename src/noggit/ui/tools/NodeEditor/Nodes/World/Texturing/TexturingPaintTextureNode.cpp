// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TexturingPaintTextureNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/Brush.h>

using namespace noggit::Red::NodeEditor::Nodes;

TexturingPaintTextureNode::TexturingPaintTextureNode()
: ContextLogicNodeBase()
{
  setName("Texturing :: PaintTexture");
  setCaption("Texturing :: PaintTexture");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);

  auto radius = static_cast<QDoubleSpinBox*>(_in_ports[2].default_widget);
  radius->setMinimum(0);

  addPortDefault<DecimalData>(PortType::In, "Hardness<Decimal>", true);

  auto hardness = static_cast<QDoubleSpinBox*>(_in_ports[3].default_widget);
  hardness->setRange(0.0, 1.0);

  addPortDefault<DecimalData>(PortType::In, "AlphaTarget<Decimal>", true);

  auto alpha_target = static_cast<QDoubleSpinBox*>(_in_ports[4].default_widget);
  alpha_target->setRange(0.0, 1.0);

  addPortDefault<DecimalData>(PortType::In, "Pressure<Decimal>", true);

  auto pressure = static_cast<QDoubleSpinBox*>(_in_ports[5].default_widget);
  pressure->setRange(0.0, 1.0);

  addPortDefault<StringData>(PortType::In, "Texture<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TexturingPaintTextureNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  Brush brush;
  brush.setRadius(std::max(0.0, defaultPortData<DecimalData>(PortType::In, 2)->value()));
  brush.setHardness(std::max(0.0, std::min(1.0, defaultPortData<DecimalData>(PortType::In, 3)->value())));

  scoped_blp_texture_reference tex(defaultPortData<StringData>(PortType::In, 6)->value(),
                                   gCurrentContext->getViewport()->getRenderContext());

  world->paintTexture({pos.x, pos.y, pos.z}, &brush,
                      std::max(0.0, std::min(1.0, defaultPortData<DecimalData>(PortType::In, 4)->value())),
                      std::max(0.0, std::min(1.0, defaultPortData<DecimalData>(PortType::In, 5)->value())),
                      tex);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
