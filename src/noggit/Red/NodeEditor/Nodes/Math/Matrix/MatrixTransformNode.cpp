// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixTransformNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/glm/gtx/transform.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

MatrixTransformNode::MatrixTransformNode()
: BaseNode()
{
  setName("Matrix :: Transform");
  setCaption("Matrix :: Transform");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Translate", "Rotate Euler", "Scale"});

  addWidgetTop(_operation);

  addPort<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPort<Vector3DData>(PortType::In, "Vector3D", true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 0);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixTransformNode::compute()
{
  glm::mat4 matrix = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value();
  glm::vec3 transform_vec = static_cast<Vector3DData*>(_in_ports[1].in_value.lock().get())->value();

  switch (_operation->currentIndex())
  {
    case 0: // Translate
      matrix = glm::translate(matrix, transform_vec);
      break;

    case 1: // Rotate Euler
      matrix = glm::rotate(matrix, glm::radians(transform_vec[0]), glm::vec3(1.0, 0.0, 0.0));
      matrix = glm::rotate(matrix, glm::radians(transform_vec[1]), glm::vec3(0.0, 1.0, 0.0));
      matrix = glm::rotate(matrix, glm::radians(transform_vec[2]), glm::vec3(0.0, 0.0, 1.0));
      break;

    case 2: // Scale
      matrix = glm::scale(matrix, transform_vec);
      break;
  }

  _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(matrix);
  _node->onDataUpdated(0);
}

QJsonObject MatrixTransformNode::save() const
{
  QJsonObject json_obj = BaseNode::save();
  json_obj["operation"] = _operation->currentIndex();
  _in_ports[1].data_type->to_json(_in_ports[1].default_widget, json_obj, "transform_vector");
  return json_obj;
}

void MatrixTransformNode::restore(const QJsonObject& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());
  BaseNode::restore(json_obj);
  _in_ports[1].data_type->from_json(_in_ports[1].default_widget, json_obj, "transform_vector");
}

NodeValidationState MatrixTransformNode::validate()
{
  if (!_in_ports[0].in_value.lock() || !_in_ports[1].in_value.lock())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate input.");
    return _validation_state;
  }

  return _validation_state;
}
