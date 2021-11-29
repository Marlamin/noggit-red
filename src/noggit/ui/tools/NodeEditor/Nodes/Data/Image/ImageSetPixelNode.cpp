// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageSetPixelNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

ImageSetPixelNode::ImageSetPixelNode()
: LogicNodeBase()
{
  setName("Image :: SetPixel");
  setCaption("Image :: SetPixel");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<Vector2DData>(PortType::In, "PixelXY<Vector2D>", true);
  addPortDefault<ColorData>(PortType::In, "Color", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageSetPixelNode::compute()
{
  QImage* image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value_ptr();
  glm::vec2 pixel_xy = defaultPortData<Vector2DData>(PortType::In, 2)->value();
  glm::vec4 color = defaultPortData<ColorData>(PortType::In, 3)->value();

  if (pixel_xy.x >= image->width() || pixel_xy.y >= image->height() || pixel_xy.y < 0 || pixel_xy.x < 0)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: pixel coordinates are out of range.");
    return;
  }

  image->setPixelColor(pixel_xy.x, pixel_xy.y, QColor::fromRgbF(color.r, color.g, color.b, color.a));

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = _in_ports[1].in_value.lock();
  _node->onDataUpdated(1);
}

NodeValidationState ImageSetPixelNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject ImageSetPixelNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "pixel_xy");
  defaultWidgetToJson(PortType::In, 3, json_obj, "color");

  return json_obj;
}

void ImageSetPixelNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "pixel_xy");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "color");
}
