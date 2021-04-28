// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetAlphaLayer.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileGetAlphaLayerNode::TileGetAlphaLayerNode()
: ContextLogicNodeBase()
{
  setName("Tile :: GetAlphaLayer");
  setCaption("Tile :: GetAlphaLayer");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Layer<UInteger>", true);
  auto layer = static_cast<QSpinBox*>(_in_ports[2].default_widget);
  layer->setRange(1, 3);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void TileGetAlphaLayerNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  unsigned layer = defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value();

  if (layer < 1 || layer > 3)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: layer is out of range.");
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(tile->getAlphamapImage(layer));
  _node->onDataUpdated(1);

}


NodeValidationState TileGetAlphaLayerNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
   setValidationState(NodeValidationState::Error);
   setValidationMessage("Error: failed to evaluate tile input.");
   return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}


