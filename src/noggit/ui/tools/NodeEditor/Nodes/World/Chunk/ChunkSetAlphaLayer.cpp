// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkSetAlphaLayer.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkSetAlphaLayerNode::ChunkSetAlphaLayerNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: SetAlphaLayer");
  setCaption("Chunk :: SetAlphaLayer");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Layer<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ChunkSetAlphaLayerNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  QImage* image = defaultPortData<ImageData>(PortType::In, 2)->value_ptr();

  if (image->width() != 64 || image->height() != 64)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: image should have a 64x64 resolution.");
    return;
  }

  auto texture_set = chunk->getTextureSet();

  if (!texture_set)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: chunk does not have any textures.");
    return;
  }

  unsigned int layer = defaultPortData<UnsignedIntegerData>(PortType::In, 3)->value();

  if (!layer)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: layer 0 cannot have an alpha layer.");
    return;
  }

  if (layer < 1 || layer > 3 || layer >= texture_set->num())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: layer is out of range.");
    return;
  }

  texture_set->create_temporary_alphamaps_if_needed();
  auto& temp_alphamaps = texture_set->getTempAlphamaps()->get();

  for (int i = 0; i < 64; ++i)
  {
    for (int j = 0; j < 64; ++j)
    {
      temp_alphamaps[layer][64 * j + i] = static_cast<float>(qGray(image->pixel(i, j))) / 255.0f;
    }
  }

  texture_set->markDirty();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);


}

NodeValidationState ChunkSetAlphaLayerNode::validate()
{

  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  if (!static_cast<ImageData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}


