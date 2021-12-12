// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetMinMaxHeight.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

TileGetMinMaxHeightNode::TileGetMinMaxHeightNode()
: ContextNodeBase()
{
  setName("Tile :: GetMinMaxHeight");
  setCaption("Tile :: GetMinMaxHeight");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<DecimalData>(PortType::Out, "MinHeight<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "MaxHeight<Decimal>", true);
}

void TileGetMinMaxHeightNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<DecimalData>(tile->getMinHeight());
  _node->onDataUpdated(1);

  _out_ports[2].out_value = std::make_shared<DecimalData>(tile->getMaxHeight());
  _node->onDataUpdated(2);

}


NodeValidationState TileGetMinMaxHeightNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextNodeBase::validate();

}

