// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkSwapTexture.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkSwapTextureNode::ChunkSwapTextureNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: SwapTexture");
  setCaption("Chunk :: SwapTexture");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<StringData>(PortType::In, "TextureFrom<String>", true);
  addPortDefault<StringData>(PortType::In, "TextureTo<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ChunkSwapTextureNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  auto tex_from = defaultPortData<StringData>(PortType::In, 2)->value();
  auto tex_to = defaultPortData<StringData>(PortType::In, 3)->value();

  if (tex_from.empty() || tex_to.empty())
    return;

  scoped_blp_texture_reference b_tex_from(tex_from, gCurrentContext->getViewport()->getRenderContext());
  scoped_blp_texture_reference b_tex_to(tex_to, gCurrentContext->getViewport()->getRenderContext());

  chunk->switchTexture(b_tex_from, b_tex_to);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState ChunkSwapTextureNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
