// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageResizeNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/NodeEditor/include/nodes/Node>

#include <QComboBox>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageResizeNode::ImageResizeNode()
: LogicNodeBase()
{
  setName("Image :: Resize");
  setCaption("Image :: Resize");
  _validation_state = NodeValidationState::Valid;

  _mode = new QComboBox(&_embedded_widget);
  _mode->addItems({"Fast", "Smooth"});
  addWidgetTop(_mode);

  _aspect_ratio_mode = new QComboBox(&_embedded_widget);
  _aspect_ratio_mode->addItems({"Ignore", "Keep", "Keep by expanding"});
  addWidgetTop(_aspect_ratio_mode);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<Vector2DData>(PortType::In, "Size<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);


}

void ImageResizeNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value();
  glm::vec2 size = defaultPortData<Vector2DData>(PortType::In, 2)->value();
  QImage new_img = image.scaled(size.x, size.y,
                static_cast<Qt::AspectRatioMode>(_aspect_ratio_mode->currentIndex()),
                     static_cast<Qt::TransformationMode>(_mode->currentIndex()));

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(new_img));
  _node->onDataUpdated(1);

}

NodeValidationState ImageResizeNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

   return LogicNodeBase::validate();
}

QJsonObject ImageResizeNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "size");

  return json_obj;
}

void ImageResizeNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  defaultWidgetFromJson(PortType::In, 2, json_obj, "size");
}
