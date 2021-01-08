// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixRotateEulerNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"
#include <external/glm/gtx/transform.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

MatrixRotateEulerNode::MatrixRotateEulerNode()
: BaseNode()
{
  setName("MatrixRotateEulerNode");
  setCaption("Matrix Rotate Euler");
  _validation_state = NodeValidationState::Valid;

  addPort<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPort<Vector3DData>(PortType::In, "Vector3D", true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 0);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixRotateEulerNode::compute()
{
  glm::mat4 matrix = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value();
  glm::vec3 rot_vector = static_cast<Vector3DData*>(_in_ports[1].in_value.lock().get())->value();

  matrix = glm::rotate(matrix, glm::radians(rot_vector[0]), glm::vec3(1.0, 0.0, 0.0));
  matrix = glm::rotate(matrix, glm::radians(rot_vector[1]), glm::vec3(0.0, 1.0, 0.0));
  matrix = glm::rotate(matrix, glm::radians(rot_vector[2]), glm::vec3(0.0, 0.0, 1.0));

  _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(matrix);
  Q_EMIT dataUpdated(0);
}

QJsonObject MatrixRotateEulerNode::save() const
{
  QJsonObject json_obj = BaseNode::save();
  _in_ports[1].data_type->to_json(_in_ports[1].default_widget, json_obj, "rotation_vector");
  return json_obj;
}

void MatrixRotateEulerNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  _in_ports[1].data_type->from_json(_in_ports[1].default_widget, json_obj, "rotation_vector");
}

NodeValidationState MatrixRotateEulerNode::validate()
{
  if (!_in_ports[0].in_value.lock() || !_in_ports[1].in_value.lock())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate input.");
    return _validation_state;
  }

  return _validation_state;
}

