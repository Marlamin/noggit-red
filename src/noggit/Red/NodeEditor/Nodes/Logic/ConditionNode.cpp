#include "ConditionNode.hpp"
#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <cmath>
#include <limits>

using namespace noggit::Red::NodeEditor::Nodes;

ConditionNode::ConditionNode()
: BaseNode()
{
  setName("Logic :: Condition");
  setCaption("Equal");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Equal",
                        "Not equal",
                        "Less",
                        "Greater",
                        "And",
                        "Or",
                        "Xor"});
  addWidgetTop(_operation);

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
                     {
                         setCaption(_operation->currentText());
                     }
  );

  addPort<DecimalData>(PortType::In, "Value", true);
  _first = new QDoubleSpinBox(&_embedded_widget);
  addDefaultWidget(_first, PortType::In, 0);

  addPort<DecimalData>(PortType::In, "Value", true);
  _second = new QDoubleSpinBox(&_embedded_widget);
  addDefaultWidget(_second, PortType::In, 1);

  addPort<BooleanData>(PortType::Out, "Boolean", true);
}

void ConditionNode::compute()
{
  auto first = static_cast<DecimalData*>(_in_ports[0].in_value.lock().get());
  auto second = static_cast<DecimalData*>( _in_ports[1].in_value.lock().get());

  // handle defaults
  double first_number = first ? first->value() : _first->value();
  double second_number = second ? second->value() : _second->value();

  setValidationState(NodeValidationState::Warning);

  bool result = false;

  switch (_operation->currentIndex())
  {
    case 0:
      result = std::fabs(std::fabs(first_number) - std::fabs(second_number)) < std::numeric_limits<double>::epsilon();
      break;
    case 1:
      result = first_number != second_number;
      break;
    case 2:
      result = first_number < second_number;
      break;
    case 3:
      result = first_number > second_number;
      break;
    case 4:
      result = static_cast<bool>(first_number) && static_cast<bool>(second_number);
      break;
    case 5:
      result = static_cast<bool>(first_number) || static_cast<bool>(second_number);
      break;
    case 6:
      result = static_cast<bool>(first_number) ^ static_cast<bool>(second_number);
      break;
  }

  _out_ports[0].out_value = std::make_shared<BooleanData>(result);

  setValidationMessage(("Debug: " + std::to_string(result)).c_str());

  Q_EMIT dataUpdated(0);
}

QJsonObject ConditionNode::save() const
{
  QJsonObject json_obj;

  json_obj["name"] = name();
  json_obj["caption"] = caption();
  json_obj["default_first"] = _first->value();
  json_obj["default_second"] = _second->value();
  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void ConditionNode::restore(const QJsonObject& json_obj)
{
  setName(json_obj["name"].toString());
  setCaption(json_obj["caption"].toString());
  _first->setValue(json_obj["default_first"].toDouble());
  _second->setValue(json_obj["default_second"].toDouble());
  _operation->setCurrentIndex(json_obj["operation"].toInt());
}
