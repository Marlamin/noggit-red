// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::NodeEditor::Nodes;

MatrixNode::MatrixNode()
: BaseNode()
{
  setName("MatrixNode");
  setCaption("Matrix");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Identity", "Zero"});

  addWidgetTop(_operation);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixNode::compute()
{
  _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(_operation->currentIndex() ? glm::mat4(0.0) : glm::mat4(1.0));
  Q_EMIT dataUpdated(0);
}

QJsonObject MatrixNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void MatrixNode::restore(const QJsonObject& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());
  BaseNode::restore(json_obj);
}

