// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageGetPixelNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageGetPixelNode::ImageGetPixelNode()
: LogicNodeBase()
{
  setName("ImageGetPixelNode");
  setCaption("Image Get Pixel");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<Vector2DData>(PortType::In, "PixelXY<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ColorData>(PortType::Out, "Color", true);
}

void ImageGetPixelNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value();
  glm::vec2 pixel_xy = defaultPortData<Vector2DData>(PortType::In, 2)->value();

  if (pixel_xy.x >= image.width() || pixel_xy.y >= image.height() || pixel_xy.y < 0 || pixel_xy.x < 0)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: pixel coordinates are out of range.");
    return;
  }

  QColor color = image.pixelColor(pixel_xy.x, pixel_xy.y);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ColorData>(glm::vec4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
  Q_EMIT dataUpdated(1);
}

NodeValidationState ImageGetPixelNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

   return LogicNodeBase::validate();
}

QJsonObject ImageGetPixelNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "pixel_xy");

  return json_obj;
}

void ImageGetPixelNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "pixel_xy");
}

