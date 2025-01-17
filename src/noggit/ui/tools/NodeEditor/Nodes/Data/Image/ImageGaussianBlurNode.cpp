// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageGaussianBlurNode.hpp"
#include "gaussianblur.h"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageGaussianBlurNode::ImageGaussianBlurNode()
: LogicNodeBase()
{
  setName("Image :: GaussianBlur");
  setCaption("Image :: GaussianBlur");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<DecimalData>(PortType::In, "Sigma<Decimal>", true);
  addPortDefault<UnsignedIntegerData>(PortType::In, "Passes<UInteger>", true);
  auto passes = static_cast<QSpinBox*>(_in_ports[3].default_widget);
  passes->setValue(1);
  passes->setMinimum(1);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageGaussianBlurNode::compute()
{
  QImage* image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value_ptr();

  QImage new_image = *image;
  for (unsigned i = 0; i < defaultPortData<UnsignedIntegerData>(PortType::In, 3)->value(); ++i)
  {
    GaussianBlur blur(5.0,
                      std::max(defaultPortData<DecimalData>(PortType::In, 2)->value(), 1.0));

    new_image = blur.ApplyGaussianFilterToImage(new_image);
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(new_image));
  _node->onDataUpdated(1);

}

NodeValidationState ImageGaussianBlurNode::validate()
{

  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject ImageGaussianBlurNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "sigma");
  defaultWidgetToJson(PortType::In, 3, json_obj, "passes");

  return json_obj;
}

void ImageGaussianBlurNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "sigma");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "passes");

}
