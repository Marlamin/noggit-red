// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileSetVertexColorsImage.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

TileSetVertexColorsImageNode::TileSetVertexColorsImageNode()
: ContextLogicNodeBase()
{
  setName("Tile :: SetVertexColorsImage");
  setCaption("Tile :: SetVertexColorsImage");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Set", "Add", "Subtract", "Multiply"});
  addWidgetTop(_operation);

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<TileData>(PortType::In, "Logic", true);
  addPort<ImageData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TileSetVertexColorsImageNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  QImage* image = defaultPortData<ImageData>(PortType::In, 2)->value_ptr();

  QImage* image_to_use = image;

  if (image->width() != image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: image should have square dimensions.");
    return;
  }

  QImage scaled;
  if (image->width() != 257)
  {
    scaled = image->scaled(257, 257, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image_to_use = &scaled;
  }

  tile->setVertexColorImage(*image_to_use, _operation->currentIndex(), false);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState TileSetVertexColorsImageNode::validate()
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

