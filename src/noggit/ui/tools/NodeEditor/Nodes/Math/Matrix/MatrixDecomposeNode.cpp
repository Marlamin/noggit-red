// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixDecomposeNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/NodeEditor/include/nodes/Node>

#include <external/glm/gtx/matrix_decompose.hpp>
#include <external/glm/gtx/euler_angles.hpp>
#include <external/glm/gtx/quaternion.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

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
  addPort<Vector3DData>(PortType::Out, "Orientation<Vector3D>", true);
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

  if (_out_ports[0].connected)
  {
    _out_ports[0].out_value = std::make_shared<Vector3DData>(translation);
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    _out_ports[1].out_value = std::make_shared<Vector3DData>(scale);
    _node->onDataUpdated(1);
  }

  if (_out_ports[2].connected)
  {
    _out_ports[2].out_value = std::make_shared<QuaternionData>(orientation);
    _node->onDataUpdated(2);
  }

  if (_out_ports[3].connected)
  {
    _out_ports[3].out_value = std::make_shared<Vector3DData>(skew);
    _node->onDataUpdated(3);
  }

  if (_out_ports[4].connected)
  {
    _out_ports[4].out_value = std::make_shared<Vector3DData>(perspective);
    _node->onDataUpdated(4);
  }

  if (_out_ports[5].connected)
  {

    glm::mat4 rot_matrix = glm::toMat4(orientation);

    float x;
    float y;
    float z;

    extractEulerAngleYZX(matrix, z, y, x);

    _out_ports[5].out_value = std::make_shared<Vector3DData>(glm::vec3(glm::degrees(x),
                                                                       glm::degrees(y),
                                                                       glm::degrees(z)));
    _node->onDataUpdated(5);
  }

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
