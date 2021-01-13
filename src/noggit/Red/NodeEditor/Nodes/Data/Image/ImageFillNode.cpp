// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageFillNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageFillNode::ImageFillNode()
: LogicNodeBase()
{
  setName("ImageFillNode");
  setCaption("Image Fill");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<ColorData>(PortType::In, "Color", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageFillNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value();
  glm::vec4 color = defaultPortData<ColorData>(PortType::In, 2)->value();
  image.fill(QColor::fromRgbF(color.r, color.g, color.b, color.a));

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = _in_ports[1].in_value.lock();
  Q_EMIT dataUpdated(1);
}

NodeValidationState ImageFillNode::validate()
{

  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject ImageFillNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "color");

  return json_obj;
}

void ImageFillNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "color");
}

