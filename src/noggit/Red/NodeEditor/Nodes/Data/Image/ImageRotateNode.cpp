// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageRotateNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageRotateNode::ImageRotateNode()
: LogicNodeBase()
{
  setName("Image :: Rotate");
  setCaption("Image :: Rotate");
  _validation_state = NodeValidationState::Valid;

  _mode = new QComboBox(&_embedded_widget);
  _mode->addItems({"Fast", "Smooth"});
  addWidgetTop(_mode);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<DecimalData>(PortType::In, "Angle<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageRotateNode::compute()
{
  double angle = defaultPortData<DecimalData>(PortType::In, 2)->value();

  _out_ports[1].out_value = std::make_shared<ImageData>(
      std::move(static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value().transformed(
          QTransform().rotate(angle), static_cast<Qt::TransformationMode>(_mode->currentIndex()))));
  Q_EMIT dataUpdated(1);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);
}


QJsonObject ImageRotateNode::save() const
{
  QJsonObject json_obj = BaseNode::save();
  json_obj["mode"] = _mode->currentIndex();
  defaultWidgetToJson(PortType::In, 2, json_obj, "angle");
  return json_obj;
}

void ImageRotateNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  defaultWidgetFromJson(PortType::In, 2, json_obj, "angle");
  _mode->setCurrentIndex(json_obj["mode"].toInt());
}

NodeValidationState ImageRotateNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}
