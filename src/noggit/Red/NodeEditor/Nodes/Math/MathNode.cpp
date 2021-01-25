#include "MathNode.hpp"
#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/Red/NodeEditor/Nodes/Scene/NodeScene.hpp>
#include <external/NodeEditor/include/nodes/Node>
#include <cmath>
#include <stdexcept>

using namespace noggit::Red::NodeEditor::Nodes;
using QtNodes::Node;


MathNode::MathNode()
: BaseNode()
{
  _validation_state = NodeValidationState::Valid;
  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Add",
                        "Subtract",
                        "Multiply",
                        "Divide",
                        "Modulo",
                        "Min",
                        "Max",
                        "Pow"});

  addWidgetTop(_operation);

  setName("MathNode");
  setCaption("Add");

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
          ,[this](int index)
          {
            setCaption(_operation->currentText());
          }
  );

  addPortDefault<DecimalData>(PortType::In, "Decimal", true);

  addPortDefault<DecimalData>(PortType::In, "Decimal", true);

  addPort<DecimalData>(PortType::Out, "Decimal", true);
}

void MathNode::compute()
{

  double result;
  double first = defaultPortData<DecimalData>(PortType::In, 0)->value();
  double second = defaultPortData<DecimalData>(PortType::In, 1)->value();

  switch (_operation->currentIndex())
  {
    case 0: // Add
      result = first + second;
      break;
    case 1: // Subtract
      result = first - second;
      break;
    case 2: // Multiply
      result = first * second;
      break;
    case 3: // Divide
      result = first / second;
      break;
    case 4: // Module
      result = std::fmod(first, second);
      break;
    case 5: // Min
      result = std::min(first, second);
      break;
    case 6: // Max
      result = std::max(first, second);
      break;
    case 7: // Pow
      result = std::pow(first, second);
      break;
  }

  _out_ports[0].out_value = std::make_shared<DecimalData>(result);

  _node->onDataUpdated(0);

}

QJsonObject MathNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void MathNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  _operation->setCurrentIndex(json_obj["operation"].toInt());
}


