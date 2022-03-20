// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageBlendOpenGLNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageBlendOpenGLNode::ImageBlendOpenGLNode()
: LogicNodeBase()
{
  setName("Image :: BlendOpenGL");
  setCaption("Image :: BlendOpenGL");
  _validation_state = NodeValidationState::Valid;

  _blend_func = new QComboBox(&_embedded_widget);
  _blend_func->addItems({"Add", "Subtract", "Reverse Subtract", "Min", "Max"});
  addWidgetTop(_blend_func);

  _sfactor = new QComboBox(&_embedded_widget);
  _sfactor->addItems({"ZERO",
                         "ONE",
                         "SRC_COLOR",
                         "ONE_MINUS_SRC_COLOR",
                         "DST_COLOR",
                         "ONE_MINUS_DST_COLOR",
                         "SRC_ALPHA",
                         "ONE_MINUS_SRC_ALPHA",
                         "DST_ALPHA",
                         "ONE_MINUS_DST_ALPHA"});
  addWidgetTop(_sfactor);

  _dfactor = new QComboBox(&_embedded_widget);
  _dfactor->addItems({"GL_ZERO",
                      "ONE",
                      "SRC_COLOR",
                      "ONE_MINUS_SRC_COLOR",
                      "DST_COLOR",
                      "ONE_MINUS_DST_COLOR",
                      "SRC_ALPHA",
                      "ONE_MINUS_SRC_ALPHA",
                      "DST_ALPHA",
                      "ONE_MINUS_DST_ALPHA"});
  addWidgetTop(_dfactor);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageBlendOpenGLNode::compute()
{
  QImage* source_img = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value_ptr();
  QImage* dest_img = static_cast<ImageData*>(_in_ports[2].in_value.lock().get())->value_ptr();

  if (source_img->width() != dest_img->width() || source_img->height() != dest_img->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: images have different resolution.");
    return;
  }

  QImage result_image = QImage(dest_img->size(), dest_img->format());

  for (int i = 0; i < result_image.width(); ++i)
  {
    for (int j = 0; j < result_image.height(); ++j)
    {
      result_image.setPixelColor(i, j, blendPixels(source_img->pixelColor(i, j), dest_img->pixelColor(i, j)));
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(result_image));
  _node->onDataUpdated(1);
}

QColor ImageBlendOpenGLNode::blendPixels(QColor const& source, QColor const& dest)
{
  QColor source_fac = computeFactor(source, dest, _sfactor->currentIndex());
  QColor dest_fac = computeFactor(source, dest, _dfactor->currentIndex());

  QColor result;
  switch(_blend_func->currentIndex())
  {
    case 0: // Add
    {
      result.setRedF(std::max(0.0, std::min(1.0, source.redF() * source_fac.redF() + dest.redF() * dest_fac.redF())));
      result.setGreenF(std::max(0.0, std::min(1.0, source.greenF() * source_fac.greenF() + dest.greenF() * dest_fac.greenF())));
      result.setBlueF(std::max(0.0, std::min(1.0, source.blueF() * source_fac.blueF() + dest.blueF() * dest_fac.blueF())));
      result.setAlphaF(std::max(0.0, std::min(1.0, source.alphaF() * source_fac.alphaF() + dest.alphaF() * dest_fac.alphaF())));
      break;
    }
    case 1: // Subtract
    {
      result.setRedF(std::max(0.0, std::min(1.0, source.redF() * source_fac.redF() - dest.redF() * dest_fac.redF())));
      result.setGreenF(std::max(0.0, std::min(1.0, source.greenF() * source_fac.greenF() - dest.greenF() * dest_fac.greenF())));
      result.setBlueF(std::max(0.0, std::min(1.0, source.blueF() * source_fac.blueF() - dest.blueF() * dest_fac.blueF())));
      result.setAlphaF(std::max(0.0, std::min(1.0, source.alphaF() * source_fac.alphaF() - dest.alphaF() * dest_fac.alphaF())));
      break;
    }
    case 2: // Reverse Subtract
    {
      result.setRedF(std::max(0.0, std::min(1.0, dest.redF() * dest_fac.redF() - source.redF() * source_fac.redF())));
      result.setGreenF(std::max(0.0, std::min(1.0, dest.greenF() * dest_fac.greenF() - source.greenF() * source_fac.greenF())));
      result.setBlueF(std::max(0.0, std::min(1.0, dest.blueF() * dest_fac.blueF() - source.blueF() * source_fac.blueF())));
      result.setAlphaF(std::max(0.0, std::min(1.0, dest.alphaF() * dest_fac.alphaF() - source.alphaF() * source_fac.alphaF())));
      break;
    }
    case 3: // Min
    {
      result.setRedF(std::max(0.0, std::min(1.0, std::min(source.redF() * source_fac.redF(), dest.redF() * dest_fac.redF()))));
      result.setGreenF(std::max(0.0, std::min(1.0,  std::min(source.greenF() * source_fac.greenF(), dest.greenF() * dest_fac.greenF()))));
      result.setBlueF(std::max(0.0, std::min(1.0,  std::min(source.blueF() * source_fac.blueF(), dest.blueF() * dest_fac.blueF()))));
      result.setAlphaF(std::max(0.0, std::min(1.0,  std::min(source.alphaF() * source_fac.alphaF(), dest.alphaF() * dest_fac.alphaF()))));
      break;
    }
    case 4: // Max
    {
      result.setRedF(std::max(0.0, std::min(1.0, std::max(source.redF() * source_fac.redF(), dest.redF() * dest_fac.redF()))));
      result.setGreenF(std::max(0.0, std::min(1.0,  std::max(source.greenF() * source_fac.greenF(), dest.greenF() * dest_fac.greenF()))));
      result.setBlueF(std::max(0.0, std::min(1.0,  std::max(source.blueF() * source_fac.blueF(), dest.blueF() * dest_fac.blueF()))));
      result.setAlphaF(std::max(0.0, std::min(1.0,  std::max(source.alphaF() * source_fac.alphaF(), dest.alphaF() * dest_fac.alphaF()))));
      break;
    }
  }

  return std::move(result);
}

QColor ImageBlendOpenGLNode::computeFactor(QColor const& source, QColor const& dest, int mode)
{
  QColor result;

  switch (mode)
  {
    case 0: // GL_ZERO
    {
      result = QColor::fromRgbF(0, 0, 0, 0);
      break;
    }
    case 1: // GL_ONE
    {
      result = QColor::fromRgbF(1.0, 1.0, 1.0, 1.0);
      break;
    }
    case 2: // GL_SRC_COLOR
    {
      result = source;
      break;
    }
    case 3: // GL_ONE_MINUS_SRC_COLOR
    {
      result = QColor::fromRgbF(1.0 - source.redF(),
                                1.0 - source.greenF(),
                                1.0 - source.blueF(),
                                1.0 - source.alphaF());
      break;
    }
    case 4: // GL_DST_COLOR
    {
      result = dest;
      break;
    }
    case 5: // GL_ONE_MINUS_DST_COLOR
    {
      result = QColor::fromRgbF(1.0 - dest.redF(),
                                1.0 - dest.greenF(),
                                1.0 - dest.blueF(),
                                1.0 - dest.alphaF());
      break;
    }
    case 6: // GL_SRC_ALPHA
    {
      result = QColor::fromRgbF(source.alphaF(), source.alphaF(), source.alphaF(), source.alphaF());
      break;
    }
    case 7: // GL_ONE_MINUS_SRC_ALPHA
    {
      result = QColor::fromRgbF(1.0 - source.alphaF(),
                                1.0 - source.alphaF(),
                                1.0 - source.alphaF(),
                                1.0 - source.alphaF());
      break;
    }
    case 8: // GL_DST_ALPHA
    {
      result = QColor::fromRgbF(dest.alphaF(), dest.alphaF(), dest.alphaF(), dest.alphaF());
      break;
    }
    case 9: // GL_ONE_MINUS_DST_ALPHA
    {
      result = QColor::fromRgbF(1.0 - dest.alphaF(),
                                1.0 - dest.alphaF(),
                                1.0 - dest.alphaF(),
                                1.0 - dest.alphaF());
      break;
    }
  }

  return std::move(result);
}

NodeValidationState ImageBlendOpenGLNode::validate()
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

QJsonObject ImageBlendOpenGLNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["blend_func"] = _blend_func->currentIndex();
  json_obj["s_factor"] = _sfactor->currentIndex();
  json_obj["d_factor"] = _dfactor->currentIndex();

  return json_obj;
}

void ImageBlendOpenGLNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  _blend_func->setCurrentIndex(json_obj["blend_func"].toInt());
  _sfactor->setCurrentIndex(json_obj["s_factor"].toInt());
  _dfactor->setCurrentIndex(json_obj["d_factor"].toInt());
}
