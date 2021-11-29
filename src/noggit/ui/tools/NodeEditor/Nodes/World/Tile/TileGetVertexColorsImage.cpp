// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetVertexColorsImage.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

TileGetVertexColorsImageNode::TileGetVertexColorsImageNode()
: ContextLogicNodeBase()
{
  setName("Tile :: GetVertexColorsImage");
  setCaption("Tile :: GetVertexColorsImage");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<TileData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void TileGetVertexColorsImageNode::compute()
{
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(tile->getVertexColorsImage());
  _node->onDataUpdated(1);

}


NodeValidationState TileGetVertexColorsImageNode::validate()
{

  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

