// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/NodeEditor/include/nodes/Node>

#include <QComboBox>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

MatrixNode::MatrixNode()
: BaseNode()
{
  setName("Matrix :: Create");
  setCaption("Matrix :: Create");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Identity", "Zero"});

  addWidgetTop(_operation);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixNode::compute()
{
  _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(_operation->currentIndex() ? glm::mat4(0.0) : glm::mat4(1.0));
  _node->onDataUpdated(0);
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
