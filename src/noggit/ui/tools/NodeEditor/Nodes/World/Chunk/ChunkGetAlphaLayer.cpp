// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkGetAlphaLayer.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkGetAlphaLayerNode::ChunkGetAlphaLayerNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: GetAlphaLayer");
  setCaption("Chunk :: GetAlphaLayer");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Layer<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ChunkGetAlphaLayerNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();

  auto texture_set = chunk->getTextureSet();

  if (!texture_set)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: chunk does not have any textures.");
    return;
  }

  unsigned int layer = defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value();

  if (!layer)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: layer 0 cannot have an alpha layer.");
    return;
  }

  if (layer < 0 || layer > 3 || layer >= texture_set->num())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: layer is out of range.");
    return;
  }

  texture_set->apply_alpha_changes();
  auto alphamaps = texture_set->getAlphamaps();

  auto alpha_layer = alphamaps->at(layer - 1).get();

  QImage image(64, 64, QImage::Format_RGBA8888);

  for (int i = 0; i < 64; ++i)
  {
    for (int j = 0; j < 64; ++j)
    {
      int value = alpha_layer.getAlpha(64 * j + i);
      image.setPixelColor(i, j, QColor(value, value, value, 255));
    }
  }


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(image));
  _node->onDataUpdated(1);

}

NodeValidationState ChunkGetAlphaLayerNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}


