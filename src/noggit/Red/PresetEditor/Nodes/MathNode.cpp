#include "MathNode.hpp"
#include "BaseNode.inl"
#include "Data/DecimalData.hpp"
#include <cmath>

using namespace noggit::Red::PresetEditor::Nodes;

MathNode::MathNode()
: BaseNode()
{
  _validation_state = NodeValidationState::Valid;
  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Add",
                        "Subtract",
                        "Multiply",
                        "Divide",
                        "Modulo"});
  addWidget(_operation);

  setName("MathNode");
  setCaption("Add");

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
          ,[this](int index)
          {
              setCaption(_operation->currentText());
          }
  );

  addPort<DecimalData>(PortType::In, "Value", true);
  _first = new QDoubleSpinBox(&_embedded_widget);
  addWidget(_first, 0);

  addPort<DecimalData>(PortType::In, "Value", true);
  _second = new QDoubleSpinBox(&_embedded_widget);
  addWidget(_second, 1);

  addPort<DecimalData>(PortType::Out, "Value", true);
}

void MathNode::compute()
{
  auto first_shared = _in_ports[0].in_value.lock();
  auto second_shared = _in_ports[1].in_value.lock();

  auto first = dynamic_cast<DecimalData*>(first_shared.get());
  auto second = dynamic_cast<DecimalData*>(second_shared.get());

  // handle defaults
  double first_number = first ? first->number() : _first->value();
  double second_number = second ? second->number() : _second->value();

  setValidationState(NodeValidationState::Warning);

  double result = 0.0;

  switch (_operation->currentIndex())
  {
    case 0:
      result = first_number + second_number;
      break;
    case 1:
      result = first_number - second_number;
      break;
    case 2:
      result = first_number * second_number;
      break;
    case 3:
      result = first_number / second_number;
      break;
    case 4:
      result = std::fmod(first_number, second_number);
      break;
  }

  _out_ports[0].out_value = std::make_shared<DecimalData>(result);

  setValidationMessage(("Debug: " + std::to_string(result)).c_str());

  Q_EMIT dataUpdated(0);
}

QJsonObject MathNode::save() const
{
  QJsonObject modelJson;

  modelJson["name"] = name();
  modelJson["caption"] = caption();
  modelJson["default_first"] = _first->value();
  modelJson["default_second"] = _second->value();
  modelJson["operation"] = _operation->currentIndex();

  return modelJson;
}

void MathNode::restore(const QJsonObject& json_obj)
{
  setName(json_obj["name"].toString());
  setCaption(json_obj["caption"].toString());
  _first->setValue(json_obj["default_first"].toDouble());
  _second->setValue(json_obj["default_second"].toDouble());
  _operation->setCurrentIndex(json_obj["operation"].toInt());
}

