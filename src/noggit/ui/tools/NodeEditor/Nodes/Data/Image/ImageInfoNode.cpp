// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageInfoNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

#include <QImage>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageInfoNode::ImageInfoNode()
: BaseNode()
{
  setName("Image :: Info");
  setCaption("Image :: Info");
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

  if (_out_ports[0].connected)
  {
    _out_ports[0].out_value = std::make_shared<Vector2DData>(glm::vec2(size.width(), size.height()));
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    _out_ports[1].out_value = std::make_shared<BooleanData>(image.hasAlphaChannel());
    _node->onDataUpdated(1);
  }

  if (_out_ports[2].connected)
  {
    _out_ports[2].out_value = std::make_shared<BooleanData>(image.isNull());
    _node->onDataUpdated(2);
  }

  if (_out_ports[3].connected)
  {
    _out_ports[3].out_value = std::make_shared<IntegerData>(image.depth());
    _node->onDataUpdated(3);
  }

  if (_out_ports[4].connected)
  {
    _out_ports[4].out_value = std::make_shared<BooleanData>(image.isGrayscale());
    _node->onDataUpdated(4);

  }

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
