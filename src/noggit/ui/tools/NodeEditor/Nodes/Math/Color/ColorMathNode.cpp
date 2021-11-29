// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ColorMathNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/qt-color-widgets/qt-color-widgets/color_selector.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ColorMathNode::ColorMathNode()
: BaseNode()
{
  setName("Color :: Math");
  setCaption("Color :: Math");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Mix", "Add", "Subtract", "Multiply", "Divide"});

  addWidgetTop(_operation);

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
  QColor q_color_1 = static_cast<color_widgets::ColorSelector*>(_in_ports[2].default_widget)->color();
  glm::vec4 color_1 = _in_ports[2].in_value.lock() ? static_cast<ColorData*>(_in_ports[2].in_value.lock().get())->value()
      : glm::vec4(q_color_1.redF(), q_color_1.greenF(), q_color_1.blueF(), q_color_1.alphaF());

  QColor q_color_2 = static_cast<color_widgets::ColorSelector*>(_in_ports[3].default_widget)->color();
  glm::vec4 color_2 = _in_ports[3].in_value.lock() ? static_cast<ColorData*>(_in_ports[3].in_value.lock().get())->value()
                                                : glm::vec4(q_color_2.redF(), q_color_2.greenF(), q_color_2.blueF(), q_color_2.alphaF());

  bool clamp = _in_ports[1].in_value.lock() ? static_cast<BooleanData*>(_in_ports[1].in_value.lock().get())->value()
                                            : static_cast<QCheckBox*>(_in_ports[3].default_widget)->isChecked();

  double factor = _in_ports[0].in_value.lock() ? static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())->value()
                                               : static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget)->value();
  glm::vec4 result;

  switch (_operation->currentIndex())
  {
    case 0: // Mix
      for (int i = 0; i < 4; ++i)
      {
        result[i] = color_1[i] * (1.0 - factor) + color_2[i] * factor;
      }
      break;

    case 1: // Add
      for (int i = 0; i < 4; ++i)
      {
        result[i] = color_1[i] + color_2[i] * factor;
      }
      break;

    case 2: // Subtract
      for (int i = 0; i < 4; ++i)
      {
        result[i] = color_1[i] - color_2[i] * factor;
      }
      break;

    case 3: // Multiply
      for (int i = 0; i < 4; ++i)
      {
        result[i] = color_1[i] * (color_2[i] * factor);
      }
      break;
    case 4: // Divide
      for (int i = 0; i < 4; ++i)
      {
        result[i] = color_1[i] / (color_2[i] * factor);
      }
      break;
  }

  if (clamp)
  {
    for (int i = 0; i < 4; ++i)
    {
      result[i] = std::max(std::min(result[i], 1.0f), 0.0f);
    }
  }

  _out_ports[0].out_value = std::make_shared<ColorData>(result);
  _node->onDataUpdated(0);

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
