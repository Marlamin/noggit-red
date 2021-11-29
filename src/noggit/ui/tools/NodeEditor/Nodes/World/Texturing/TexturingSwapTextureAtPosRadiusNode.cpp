// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TexturingSwapTextureAtPosRadiusNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TexturingSwapTextureAtPosRadiusNode::TexturingSwapTextureAtPosRadiusNode()
: ContextLogicNodeBase()
{
  setName("Texturing :: SwapTextureAtPosRadius");
  setCaption("Texturing :: SwapTextureAtPosRadius");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);

  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);
  auto radius = static_cast<QDoubleSpinBox*>(_in_ports[2].default_widget);
  radius->setMinimum(0);

  addPortDefault<StringData>(PortType::In, "TextureFrom<String>", true);
  addPortDefault<StringData>(PortType::In, "TextureTo<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TexturingSwapTextureAtPosRadiusNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();
  double radius = std::max(0.0, defaultPortData<DecimalData>(PortType::In, 2)->value());

  scoped_blp_texture_reference tex_from(defaultPortData<StringData>(PortType::In, 3)->value(),
                                        gCurrentContext->getViewport()->getRenderContext());


  scoped_blp_texture_reference tex_to(defaultPortData<StringData>(PortType::In, 4)->value(),
                                      gCurrentContext->getViewport()->getRenderContext());

  world->replaceTexture({pos.x, pos.y, pos.z}, radius, tex_from, tex_to);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
