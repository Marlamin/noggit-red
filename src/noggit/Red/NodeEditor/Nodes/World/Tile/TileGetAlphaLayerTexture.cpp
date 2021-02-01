// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetAlphaLayerTexture.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileGetAlphaLayerTextureNode::TileGetAlphaLayerTextureNode()
: ContextLogicNodeBase()
{
  setName("Tile :: GetAlphaLayerTexture");
  setCaption("Tile :: GetAlphaLayerTexture");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);
  addPortDefault<StringData>(PortType::In, "Texture<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void TileGetAlphaLayerTextureNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  auto tex = defaultPortData<StringData>(PortType::In, 2)->value();

  if (tex.empty())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: texture filepath is empty.");
    return;
  }

  scoped_blp_texture_reference b_tex(tex, gCurrentContext->getViewport()->getRenderContext());

  QImage image(1024, 1024, QImage::Format_RGBA8888);
  image.fill(Qt::black);

  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      MapChunk* chunk = tile->getChunk(i, j);

      int layer = chunk->texture_set->texture_id(b_tex);

      if (layer <= 0)
        continue;

      chunk->texture_set->apply_alpha_changes();
      auto alphamaps = chunk->texture_set->getAlphamaps();

      auto alpha_layer = alphamaps->at(layer - 1).get();

      for (int k = 0; k < 64; ++k)
      {
        for (int l = 0; l < 64; ++l)
        {
          int value = alpha_layer.getAlpha(64 * l + k);
          image.setPixelColor((i * 64) + k, (j * 64) + l, QColor(value, value, value, 255));
        }
      }
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(image));
  _node->onDataUpdated(1);
}


NodeValidationState TileGetAlphaLayerTextureNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
