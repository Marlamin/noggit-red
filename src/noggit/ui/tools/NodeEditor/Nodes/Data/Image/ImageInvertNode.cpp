// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageInvertNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ImageInvertNode::ImageInvertNode()
: LogicNodeBase()
{
  setName("Image :: Invert");
  setCaption("Image :: Invert");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageInvertNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value();

  image.invertPixels();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = _in_ports[1].in_value.lock();
  _node->onDataUpdated(1);

}

NodeValidationState ImageInvertNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

   return LogicNodeBase::validate();
}
