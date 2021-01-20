// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetMaxHeightNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileGetMaxHeightNode::TileGetMaxHeightNode()
: ContextLogicNodeBase()
{
  setName("TileGetMaxHeightNode");
  setCaption("Tile Get Max Height");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<DecimalData>(PortType::Out, "Height<Decimal>", true);
}

void TileGetMaxHeightNode::compute()
{
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<DecimalData>(tile->getMaxHeight());
  Q_EMIT dataUpdated(1);

}

NodeValidationState TileGetMaxHeightNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

