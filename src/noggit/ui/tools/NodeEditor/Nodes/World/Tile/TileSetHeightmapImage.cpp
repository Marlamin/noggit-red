// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileSetHeightmapImage.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

TileSetHeightmapImageNode::TileSetHeightmapImageNode()
: ContextLogicNodeBase()
{
  setName("Tile :: SetHeightmapImage");
  setCaption("Tile :: SetHeightmapImage");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Set", "Add", "Subtract", "Multiply"});
  addWidgetTop(_operation);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<DecimalData>(PortType::In, "Multiplier<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TileSetHeightmapImageNode::compute()
{
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  QImage* image = defaultPortData<ImageData>(PortType::In, 2)->value_ptr();
  double multiplier = defaultPortData<DecimalData>(PortType::In, 3)->value();

  QImage* image_to_use = image;

  if (image->width() != image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: image should have square dimensions.");
    return;
  }

  QImage scaled;
  if (image->width() != 257 || image->height() != 257)
  {
    scaled = image->scaled(257, 257, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image_to_use = &scaled;
  }

  if (image->depth() != 64 && image->depth() != 32)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: invalid image depth.");
    return;
  }

  tile->setHeightmapImage(*image_to_use, static_cast<float>(multiplier), _operation->currentIndex());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


QJsonObject TileSetHeightmapImageNode::save() const
{
  QJsonObject json_obj = ContextLogicNodeBase::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void TileSetHeightmapImageNode::restore(const QJsonObject& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());

  ContextLogicNodeBase::restore(json_obj);
}

NodeValidationState TileSetHeightmapImageNode::validate()
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

