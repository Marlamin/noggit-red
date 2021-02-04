// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageCreateNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageCreateNode::ImageCreateNode()
: LogicNodeBase()
{
  setName("Image :: Create");
  setCaption("Image :: Create");
  _validation_state = NodeValidationState::Valid;

  _image_format = new QComboBox(&_embedded_widget);
  _image_format->addItems({"32 bit (8 per channel)", "64 bit (16 per channel)"});
  addWidgetTop(_image_format);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Width<UInteger>", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Height<UInteger>", true);
  addPortDefault<ColorData>(PortType::In, "Fill<Color>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);

}

void ImageCreateNode::compute()
{
  unsigned int width = defaultPortData<UnsignedIntegerData>(PortType::In, 1)->value();
  unsigned int height = defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value();
  glm::vec4 color = defaultPortData<ColorData>(PortType::In, 3)->value();

  QImage image = QImage(QSize(width, height), _image_format->currentIndex() ?QImage::Format_RGBA64 : QImage::Format_RGBA8888);
  image.fill(QColor::fromRgbF(color.r, color.b, color.g, color.a));

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(image));
  _node->onDataUpdated(1);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

QJsonObject ImageCreateNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 1, json_obj, "width");
  defaultWidgetToJson(PortType::In, 2, json_obj, "height");
  defaultWidgetToJson(PortType::In, 3, json_obj, "fill_color");
  json_obj["format"] = _image_format->currentIndex();

  return json_obj;
}

void ImageCreateNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 1, json_obj, "width");
  defaultWidgetFromJson(PortType::In, 2, json_obj, "height");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "fill_color");
  _image_format->setCurrentIndex(json_obj["format"].toInt());
}
