// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MathUnaryNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <cmath>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

MathUnaryNode::MathUnaryNode()
: BaseNode()
{
  setName("Math :: Unary");
  setCaption("Math :: Unary");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Sin", "Cos", "Tan", "Asin", "Acos", "Atan", "Floor", "Ceil", "Log", "Sqrt"});

  addWidgetTop(_operation);

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
         setCaption(_operation->currentText());
     }
  );

  addPort<DecimalData>(PortType::In, "Value<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "Value<Decimal>", true);
}

void MathUnaryNode::compute()
{
  double value = static_cast<DecimalData*>(_in_ports[0].in_value.lock().get())->value();

  double result;

  switch (_operation->currentIndex())
  {
    case 0: // Sin
      result = std::sin(value);
      break;

    case 1: // Cos
      result = std::cos(value);
      break;

    case 2: // Tan
      result = std::tan(value);
      break;

    case 3: // Asin
      result = std::asin(value);
      break;

    case 4: // Acos
      result = std::acos(value);
      break;

    case 5: // Atan
      result = std::atan(value);
      break;

    case 6: // Floor
      result = std::floor(value);
      break;

    case 7: // Ceil
      result = std::ceil(value);
      break;

    case 8: // Log
      result = std::log(value);
      break;

    case 9: // Sqrt
      result = std::sqrt(value);
      break;
  }

  _out_ports[0].out_value = std::make_unique<DecimalData>(result);
  _node->onDataUpdated(0);

}

QJsonObject MathUnaryNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void MathUnaryNode::restore(const QJsonObject& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());
  BaseNode::restore(json_obj);
}

NodeValidationState MathUnaryNode::validate()
{
  if (!static_cast<DecimalData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: missing number input.");
    return _validation_state;
  }

  return _validation_state;
}
