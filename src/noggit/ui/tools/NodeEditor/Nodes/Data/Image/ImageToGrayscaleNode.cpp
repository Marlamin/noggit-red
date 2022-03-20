// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageToGrayscaleNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageToGrayscaleNode::ImageToGrayscaleNode()
: LogicNodeBase()
{
  setName("Image :: ToGrayscale");
  setCaption("Image :: ToGrayscale");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<ColorData>(PortType::In, "Color", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageToGrayscaleNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value();

  for (int ii = 0; ii < image.height(); ii++)
  {
    uchar* scan = image.scanLine(ii);
    int depth = 4;
    for (int jj = 0; jj < image.width(); jj++)
    {

      QRgb* rgbpixel = reinterpret_cast<QRgb*>(scan + jj*depth);
      int gray = qGray(*rgbpixel);
      *rgbpixel = QColor(gray, gray, gray).rgba();
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(image);
  _node->onDataUpdated(1);

}

NodeValidationState ImageToGrayscaleNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

   return LogicNodeBase::validate();
}
