// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixMathNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

MatrixMathNode::MatrixMathNode()
: BaseNode()
{
  setName("MatrixMathNode");
  setCaption("Multiply");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Multiply", "Add", "Subtract", "Divide"});

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
         setCaption(_operation->currentText());
     }
  );

  addWidgetTop(_operation);
  addPort<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addPort<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixMathNode::compute()
{
  glm::mat4 result;

  switch (_operation->currentIndex())
  {
    case 0: // Multiply
      result = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value()
               * static_cast<Matrix4x4Data*>(_in_ports[1].in_value.lock().get())->value();
      break;

    case 1: // Add
      result = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value()
               + static_cast<Matrix4x4Data*>(_in_ports[1].in_value.lock().get())->value();
      break;

    case 2: // Subtract
      result = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value()
               - static_cast<Matrix4x4Data*>(_in_ports[1].in_value.lock().get())->value();
      break;

    case 3: // Divide
      result = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value()
               / static_cast<Matrix4x4Data*>(_in_ports[1].in_value.lock().get())->value();
      break;
  }

  _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(result);
  Q_EMIT dataUpdated(0);

}

QJsonObject MatrixMathNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void MatrixMathNode::restore(const QJsonObject& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());
  BaseNode::restore(json_obj);
}

NodeValidationState MatrixMathNode::validate()
{
  if (!_in_ports[0].in_value.lock() || !_in_ports[1].in_value.lock())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate matrix input.");
    return _validation_state;
  }

  return _validation_state;
}

