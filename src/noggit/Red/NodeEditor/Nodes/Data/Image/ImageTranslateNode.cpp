// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageTranslateNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageTranslateNode::ImageTranslateNode()
: LogicNodeBase()
{
  setName("ImageTranslateNode");
  setCaption("Image Translate");
  _validation_state = NodeValidationState::Valid;

  _mode = new QComboBox(&_embedded_widget);
  _mode->addItems({"Fast", "Smooth"});
  addWidgetTop(_mode);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<Vector2DData>(PortType::In, "Position<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageTranslateNode::compute()
{
  glm::vec2 translate_vec = defaultPortData<Vector2DData>(PortType::In, 2)->value();

  _out_ports[1].out_value = std::make_shared<ImageData>(
      static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value().transformed(
          QTransform().translate(translate_vec.x, translate_vec.y), static_cast<Qt::TransformationMode>(_mode->currentIndex())));
  Q_EMIT dataUpdated(1);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);
}

NodeValidationState ImageTranslateNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

   return LogicNodeBase::validate();
}

QJsonObject ImageTranslateNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["mode"] = _mode->currentIndex();

  defaultWidgetToJson(PortType::In, 2, json_obj, "translate_vec");

  return json_obj;
}

void ImageTranslateNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  defaultWidgetFromJson(PortType::In, 2, json_obj, "translate_vec");
  _mode->setCurrentIndex(json_obj["mode"].toInt());
}

