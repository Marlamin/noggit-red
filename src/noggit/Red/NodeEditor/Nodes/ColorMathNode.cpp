// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ColorMathNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"
#include <external/qt-color-widgets/qt-color-widgets/color_selector.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ColorMathNode::ColorMathNode()
: BaseNode()
{
  setName("ColorMathNode");
  setCaption("Color Mix");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Mix", "Add", "Subtract", "Multiply", "Divide"});

  addPort<DecimalData>(PortType::In, "Factor<Decimal>", true);
  addDefaultWidget(_in_ports[0].data_type->default_widget(&_embedded_widget), PortType::In, 0);
  static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget)->setRange(0.0, 1.0);

  addPort<BooleanData>(PortType::In, "Clamp<Boolean>", true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 1);

  addPort<ColorData>(PortType::In, "Color", true);
  addDefaultWidget(_in_ports[2].data_type->default_widget(&_embedded_widget), PortType::In, 2);

  addPort<ColorData>(PortType::In, "Color", true);
  addDefaultWidget(_in_ports[3].data_type->default_widget(&_embedded_widget), PortType::In, 3);

  addPort<ColorData>(PortType::Out, "Color", true);
}

void ColorMathNode::compute()
{
  QColor color_1 = _in_ports[2].in_value.lock() ? static_cast<ColorData*>(_in_ports[2].in_value.lock().get())->value()
      : static_cast<color_widgets::ColorSelector*>(_in_ports[2].default_widget)->color();

  QColor color_2 = _in_ports[3].in_value.lock() ? static_cast<ColorData*>(_in_ports[3].in_value.lock().get())->value()
                                                : static_cast<color_widgets::ColorSelector*>(_in_ports[3].default_widget)->color();

  bool clamp = _in_ports[1].in_value.lock() ? static_cast<BooleanData*>(_in_ports[1].in_value.lock().get())->value()
                                            : static_cast<QCheckBox*>(_in_ports[3].default_widget)->isChecked();

  double factor = _in_ports[0].in_value.lock() ? static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())->value()
                                               : static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget)->value();
  QColor result;

  switch (_operation->currentIndex())
  {
    case 0: // Mix
      result.setRed(color_1.red() * (1.0 - factor) + color_2.red() * factor);
      result.setGreen(color_1.green() * (1.0 - factor) + color_2.green() * factor);
      result.setBlue(color_1.blue() * (1.0 - factor) + color_2.blue() * factor);
      result.setAlpha(color_1.alpha() * (1.0 - factor) + color_2.alpha() * factor);
      break;

    case 1: // Add
      result.setRed(color_1.red() + color_2.red() * factor);
      result.setGreen(color_1.green() + color_2.green() * factor);
      result.setBlue(color_1.blue() + color_2.blue() * factor);
      result.setAlpha(color_1.alpha() + color_2.alpha() * factor);
      break;

    case 2: // Subtract
      result.setRed(color_1.red() - color_2.red() * factor);
      result.setGreen(color_1.green() - color_2.green() * factor);
      result.setBlue(color_1.blue() - color_2.blue() * factor);
      result.setAlpha(color_1.alpha() - color_2.alpha() * factor);
      break;

    case 3: // Multiply
      result.setRed(color_1.red() * (color_2.red() * factor));
      result.setGreen(color_1.green() * (color_2.green() * factor));
      result.setBlue(color_1.blue() * (color_2.blue() * factor));
      result.setAlpha(color_1.alpha() * (color_2.alpha() * factor));
      break;
    case 4: // Divide
      result.setRed(color_1.red() / (color_2.red() * factor));
      result.setGreen(color_1.green() / (color_2.green() * factor));
      result.setBlue(color_1.blue() / (color_2.blue() * factor));
      result.setAlpha(color_1.alpha() / (color_2.alpha() * factor));
      break;
  }

  if (clamp)
  {
    result.setRed(std::max(std::min(result.red(), 255), 0));
    result.setGreen(std::max(std::min(result.green(), 255), 0));
    result.setBlue(std::max(std::min(result.blue(), 255), 0));
    result.setAlpha(std::max(std::min(result.alpha(), 255), 0));
  }

  _out_ports[0].out_value = std::make_shared<ColorData>(result);
  Q_EMIT dataUpdated(0);

}

QJsonObject ColorMathNode::save() const
{
  QJsonObject json_obj = BaseNode::save();
  json_obj["operation"] = _operation->currentIndex();

  for (int i = 0; i < 4; ++i)
  {
    _in_ports[i].data_type->to_json(_in_ports[i].default_widget, json_obj, "port_" + std::to_string(i));
  }

  return json_obj;
}

void ColorMathNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  _operation->setCurrentIndex(json_obj["operation"].toInt());

  for (int i = 0; i < 4; ++i)
  {
    _in_ports[i].data_type->from_json(_in_ports[i].default_widget, json_obj, "port_" + std::to_string(i));
  }

}

