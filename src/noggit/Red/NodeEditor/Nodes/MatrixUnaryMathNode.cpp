// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixUnaryMathNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

MatrixUnaryMathNode::MatrixUnaryMathNode()
: BaseNode()
{
  setName("MatrixUnaryMathNode");
  setCaption("Invert");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Invert", "Transpose"});

  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
        setCaption(_operation->currentText());
     }
  );

  addWidgetTop(_operation);
  addPort<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixUnaryMathNode::compute()
{
  glm::mat4 matrix = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value();

  switch(_operation->currentIndex())
  {
    case 0: // Invert
      _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(glm::inverse(matrix));
      break;
    case 1: // Transpose
      _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(glm::transpose(matrix));
      break;
  }

  Q_EMIT dataUpdated(0);
}

QJsonObject MatrixUnaryMathNode::save() const
{
  QJsonObject json_obj = BaseNode::save();
  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void MatrixUnaryMathNode::restore(const QJsonObject& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());
  BaseNode::restore(json_obj);
}

NodeValidationState MatrixUnaryMathNode::validate()
{
  if (!_in_ports[0].in_value.lock().get())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate matrix input.");
    return _validation_state;
  }

  return _validation_state;
}
