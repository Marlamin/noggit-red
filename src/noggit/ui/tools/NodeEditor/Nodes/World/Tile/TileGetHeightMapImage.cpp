// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetHeightMapImage.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileGetHeightmapImageNode::TileGetHeightmapImageNode()
: ContextLogicNodeBase()
{
  setName("Tile :: GetHeightmapImage");
  setCaption("Tile :: GetHeightmapImage");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);
  addPortDefault<DecimalData>(PortType::In, "MinHeight<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "MaxHeight<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void TileGetHeightmapImageNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  double min_height = defaultPortData<DecimalData>(PortType::In, 2)->value();
  double max_height = defaultPortData<DecimalData>(PortType::In, 3)->value();

  if (min_height > max_height || ((std::fabs(std::fabs(max_height) - std::fabs(min_height)) < std::numeric_limits<double>::epsilon()) && min_height > max_height))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: incorrect precision bounds.");
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(tile->getHeightmapImage(static_cast<float>(min_height),
                                                                                static_cast<float>(max_height)));
  _node->onDataUpdated(1);

}


NodeValidationState TileGetHeightmapImageNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}


