// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkFindTextureNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkFindTextureNode::ChunkFindTextureNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: FindTexture");
  setCaption("Chunk :: FindTexture");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<StringData>(PortType::In, "Texture<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<IntegerData>(PortType::Out, "Index<Integer>", true);
}

void ChunkFindTextureNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  auto tex = defaultPortData<StringData>(PortType::In, 2)->value();

  if (tex.empty())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: texture path cannot be empty.");
    return;
  }

  scoped_blp_texture_reference b_tex(tex, gCurrentContext->getViewport()->getRenderContext());

  int tex_id = -1;
  for (int i = 0; i < chunk->texture_set->num(); ++i)
  {
    int result = chunk->texture_set->texture_id(b_tex);
    if (result >= 0)
      tex_id = result;
  }


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);


  _out_ports[1].out_value = std::make_shared<IntegerData>(tex_id);
  _node->onDataUpdated(1);

}

NodeValidationState ChunkFindTextureNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}



