// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TexturingSwapTextureAtPosNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TexturingSwapTextureAtPosNode::TexturingSwapTextureAtPosNode()
: ContextLogicNodeBase()
{
  setName("TexturingSwapTextureAtPosNode");
  setCaption("Swap Texture at Pos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<StringData>(PortType::In, "TextureFrom<String>", true);
  addPortDefault<StringData>(PortType::In, "TextureTo<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TexturingSwapTextureAtPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec3 const& pos = defaultPortData<Vector3DData>(PortType::In, 1)->value();

  scoped_blp_texture_reference tex_from(defaultPortData<StringData>(PortType::In, 2)->value(),
                                   gCurrentContext->getViewport()->getRenderContext());


  scoped_blp_texture_reference tex_to(defaultPortData<StringData>(PortType::In, 3)->value(),
                                        gCurrentContext->getViewport()->getRenderContext());

  world->overwriteTextureAtCurrentChunk({pos.x, pos.y, pos.z}, tex_from, tex_to);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

}


