// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixDecomposeNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/glm/gtx/matrix_decompose.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

MatrixDecomposeNode::MatrixDecomposeNode()
: BaseNode()
{
  setName("Matrix :: Decompose");
  setCaption("Matrix :: Decompose");
  _validation_state = NodeValidationState::Valid;

  addPort<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addPort<Vector3DData>(PortType::Out, "Translation<Vector3D>", true);
  addPort<Vector3DData>(PortType::Out, "Scale<Vector3D>", true);
  addPort<QuaternionData>(PortType::Out, "Orientation<Quaternion>", true);
  addPort<Vector3DData>(PortType::Out, "Skew<Vector3D>", true);
  addPort<Vector3DData>(PortType::Out, "Perspective<Vector3D>", true);
}

void MatrixDecomposeNode::compute()
{
  glm::mat4 matrix = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value();

  glm::vec3 scale;
  glm::quat orientation;
  glm::vec3 translation;

  glm::vec3 skew;
  glm::vec4 perspective;

  glm::decompose(matrix, scale, orientation, translation, skew, perspective);

  _out_ports[0].out_value = std::make_shared<Vector3DData>(translation);
  _out_ports[1].out_value = std::make_shared<Vector3DData>(scale);
  _out_ports[2].out_value = std::make_shared<QuaternionData>(orientation);
  _out_ports[3].out_value = std::make_shared<Vector3DData>(skew);
  _out_ports[4].out_value = std::make_shared<Vector3DData>(perspective);

  Q_EMIT dataUpdated(0);
  Q_EMIT dataUpdated(1);
  Q_EMIT dataUpdated(2);
  Q_EMIT dataUpdated(3);
  Q_EMIT dataUpdated(4);
}

NodeValidationState MatrixDecomposeNode::validate()
{
  if (!_in_ports[0].in_value.lock())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate matrix input");
    return _validation_state;
  }

  return _validation_state;
}
