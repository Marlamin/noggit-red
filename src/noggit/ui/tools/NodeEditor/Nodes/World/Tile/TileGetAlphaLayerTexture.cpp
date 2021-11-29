// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetAlphaLayerTexture.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

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

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(tile->getAlphamapImage(tex));
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

