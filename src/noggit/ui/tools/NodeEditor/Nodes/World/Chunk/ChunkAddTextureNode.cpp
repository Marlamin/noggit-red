// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkAddTextureNode.hpp"

#include <noggit/texture_set.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ChunkAddTextureNode::ChunkAddTextureNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: AddTexture");
  setCaption("Chunk :: AddTexture");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<StringData>(PortType::In, "Texture<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<IntegerData>(PortType::Out, "Index<Integer>", true);
}

void ChunkAddTextureNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  auto tex = defaultPortData<StringData>(PortType::In, 2)->value();

  scoped_blp_texture_reference b_tex(tex, gCurrentContext->getViewport()->getRenderContext());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<IntegerData>(chunk->texture_set->get_texture_index_or_add(b_tex, 1));
  _node->onDataUpdated(1);

}


NodeValidationState ChunkAddTextureNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
