// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageInfoNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <QImage>

using namespace noggit::Red::NodeEditor::Nodes;

ImageInfoNode::ImageInfoNode()
: BaseNode()
{
  setName("ImageInfoNode");
  setCaption("Image Info");
  _validation_state = NodeValidationState::Valid;

  addPort<ImageData>(PortType::In, "Image", true);

  addPort<Vector2DData>(PortType::Out, "Size<Vector2D>", true);
  addPort<BooleanData>(PortType::Out, "HasAlpha<Boolean>", true);
  addPort<BooleanData>(PortType::Out, "IsNull<Boolean>", true);
  addPort<IntegerData>(PortType::Out, "Depth<Integer>", true);
  addPort<BooleanData>(PortType::Out, "IsGrayscale<Integer>", true);
}

void ImageInfoNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[0].in_value.lock().get())->value();

  auto size = image.size();
  _out_ports[0].out_value = std::make_shared<Vector2DData>(glm::vec2(size.width(), size.height()));
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<BooleanData>(image.hasAlphaChannel());
  Q_EMIT dataUpdated(1);

  _out_ports[2].out_value = std::make_shared<BooleanData>(image.isNull());
  Q_EMIT dataUpdated(2);

  _out_ports[3].out_value = std::make_shared<IntegerData>(image.depth());
  Q_EMIT dataUpdated(3);

  _out_ports[4].out_value = std::make_shared<BooleanData>(image.isGrayscale());
  Q_EMIT dataUpdated(4);


}

NodeValidationState ImageInfoNode::validate()
{

  if (!static_cast<ImageData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input");
    return _validation_state;
  }

  return _validation_state;
}

