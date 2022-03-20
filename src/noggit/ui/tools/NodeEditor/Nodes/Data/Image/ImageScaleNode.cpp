// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageScaleNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageScaleNode::ImageScaleNode()
: LogicNodeBase()
{
  setName("Image :: Scale");
  setCaption("Image :: Scale");
  _validation_state = NodeValidationState::Valid;

  _mode = new QComboBox(&_embedded_widget);
  _mode->addItems({"Fast", "Smooth"});
  addWidgetTop(_mode);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<Vector2DData>(PortType::In, "Scale<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);

}

void ImageScaleNode::compute()
{
  glm::vec2 scale_vec = defaultPortData<Vector2DData>(PortType::In, 2)->value();

  _out_ports[1].out_value = std::make_shared<ImageData>(
      std::move(static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value().transformed(
          QTransform().scale(scale_vec.x, scale_vec.y), static_cast<Qt::TransformationMode>(_mode->currentIndex()))));
  _node->onDataUpdated(1);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
}


NodeValidationState ImageScaleNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

   return LogicNodeBase::validate();
}

QJsonObject ImageScaleNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["mode"] = _mode->currentIndex();

  defaultWidgetToJson(PortType::In, 2, json_obj, "scale_vec");

  return json_obj;
}

void ImageScaleNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  defaultWidgetFromJson(PortType::In, 2, json_obj, "scale_vec");
  _mode->setCurrentIndex(json_obj["mode"].toInt());
}
