// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkEraseUnusedTextures.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkEraseUnusedTexturesNode::ChunkEraseUnusedTexturesNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: EraseUnusedTextures");
  setCaption("Chunk :: EraseUnusedTextures");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<ChunkData>(PortType::In, "Chunk", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ChunkEraseUnusedTexturesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  chunk->texture_set->eraseUnusedTextures();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState ChunkEraseUnusedTexturesNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

