// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileSetAlphaLayer.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileSetAlphaLayerNode::TileSetAlphaLayerNode()
: ContextLogicNodeBase()
{
  setName("Tile :: SetAlphaLayer");
  setCaption("Tile :: SetAlphaLayer");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Layer<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TileSetAlphaLayerNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  QImage* image = defaultPortData<ImageData>(PortType::In, 2)->value_ptr();
  QImage* image_to_process;

  unsigned layer = defaultPortData<UnsignedIntegerData>(PortType::In, 3)->value();

  if (layer < 1 || layer > 3)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: layer out of range.");
    return;
  }

  if (image->width() != image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: image should have a square resolution.");
    return;
  }

  QImage scaled;
  if (image->width() != 1024)
  {
    scaled = image->scaled(1024, 1024, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image_to_process = &scaled;
  }

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = tile->getChunk(k, l);

      if (layer >= chunk->texture_set->num())
      {
        setValidationState(NodeValidationState::Error);
        setValidationMessage("Error: layer is out of range for some chunk.");
        return;
      }

      chunk->texture_set->create_temporary_alphamaps_if_needed();
      auto& temp_alphamaps = chunk->texture_set->getTempAlphamaps()->get();

      for (int i = 0; i < 64; ++i)
      {
        for (int j = 0; j < 64; ++j)
        {
          temp_alphamaps[layer][64 * j + i] = static_cast<float>(qGray(image->pixel((k * 64) + i, (l * 64) + j))) / 255.0f;
        }
      }

      chunk->texture_set->markDirty();

    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState TileSetAlphaLayerNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
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

