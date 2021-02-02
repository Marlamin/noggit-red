// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TexturingSetAdtBaseTextureAtPosNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/World.inl>

using namespace noggit::Red::NodeEditor::Nodes;

TexturingSetAdtBaseTextureAtPosNode::TexturingSetAdtBaseTextureAtPosNode()
: ContextLogicNodeBase()
{
  setName("Texturing :: SetADTBaseTextureAtPos");
  setCaption("Texturing :: SetADTBaseTextureAtPos");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<StringData>(PortType::In, "Texture<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TexturingSetAdtBaseTextureAtPosNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  scoped_blp_texture_reference tex(defaultPortData<StringData>(PortType::In, 2)->value(),
                                   gCurrentContext->getViewport()->getRenderContext());

  world->for_all_chunks_on_tile({pos.x, pos.y, pos.z}, [&](MapChunk* chunk)
  {
      chunk->eraseTextures();
      chunk->addTexture(tex);
  });

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}
