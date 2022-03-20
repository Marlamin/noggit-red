// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkGetTextureByLayer.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ChunkGetTextureByLayerNode::ChunkGetTextureByLayerNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: GetTextureByLayer");
  setCaption("Chunk :: GetTextureByLayer");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Chunk", true);
  auto id_wgt = static_cast<QSpinBox*>(_in_ports[2].default_widget);
  id_wgt->setRange(0, 3);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<StringData>(PortType::Out, "Texture<String>", true);
}

void ChunkGetTextureByLayerNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  unsigned id = defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value();

  if (id < 0 || id > 3 || id > chunk->texture_set->num() - 1)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: layer index is out of range.");
    return;
  }

  std::string const& tex = chunk->texture_set->texture(id)->file_key().filepath();

  _out_ports[1].out_value = std::make_shared<StringData>(tex);
  _node->onDataUpdated(1);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);


}


NodeValidationState ChunkGetTextureByLayerNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

