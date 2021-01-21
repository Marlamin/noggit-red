// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageSetRegionNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageSetRegionNode::ImageSetRegionNode()
: LogicNodeBase()
{
  setName("Image :: SetRegion");
  setCaption("Image :: SetRegion");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);

  addPortDefault<Vector2DData>(PortType::In, "Pos<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageSetRegionNode::compute()
{
  QImage* image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value_ptr();
  QImage* image_to_set = static_cast<ImageData*>(_in_ports[2].in_value.lock().get())->value_ptr();

  glm::vec2 pos = defaultPortData<Vector2DData>(PortType::In, 3)->value();

  if (pos.x < 0 || pos.x >= image->width() || pos.y < 0 || pos.y >= image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: pos is out of range.");
    return;
  }

  if (image_to_set->width() + pos.x - 1 >= image->width() || image_to_set->height() + pos.y - 1 >= image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: second image dimensions are out of range.");
    return;
  }

  QImage result = *image;

  for (int i = 0; i < image_to_set->width(); ++i)
  {
    for (int j = 0; j < image_to_set->height(); ++j)
    {
      result.setPixelColor(pos.x + i, pos.y + j, image_to_set->pixelColor(i, j));
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);
  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(result));
  Q_EMIT dataUpdated(1);

}

NodeValidationState ImageSetRegionNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get())
  || !static_cast<ImageData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject ImageSetRegionNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 3, json_obj, "pos");

  return json_obj;
}

void ImageSetRegionNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 3, json_obj, "pos");
}
