// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageGetRegionNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageGetRegionNode::ImageGetRegionNode()
: LogicNodeBase()
{
  setName("Image :: GetRegion");
  setCaption("Image :: GetRegion");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);

  addPortDefault<Vector2DData>(PortType::In, "Pos<Vector2D>", true);
  addPortDefault<Vector2DData>(PortType::In, "Dimensions<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageGetRegionNode::compute()
{
  QImage* image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value_ptr();

  glm::vec2 pos = defaultPortData<Vector2DData>(PortType::In, 2)->value();
  glm::vec2 dim = defaultPortData<Vector2DData>(PortType::In, 3)->value();

  if (pos.x < 0 || pos.x >= image->width() || pos.y < 0 || pos.y >= image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: pos is out of range.");
    return;
  }

  if (dim.x < 0 || dim.x + pos.x - 1 >= image->width() || dim.y < 0 || dim.y + pos.y - 1 >= image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: dimensions are out of range.");
    return;
  }

  QImage result = QImage(dim.x, dim.y, image->format());

  for (int i = 0; i < result.width(); ++i)
  {
    for (int j = 0; j < result.height(); ++j)
    {
      result.setPixelColor(i, j, image->pixelColor(pos.x + i, pos.y + j));
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(result));
  _node->onDataUpdated(1);

}

NodeValidationState ImageGetRegionNode::validate()
{

  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject ImageGetRegionNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "pos");
  defaultWidgetToJson(PortType::In, 3, json_obj, "dimensions");

  return json_obj;
}

void ImageGetRegionNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "pos");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "dimensions");
}
