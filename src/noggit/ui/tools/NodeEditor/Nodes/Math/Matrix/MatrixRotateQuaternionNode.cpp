// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixRotateQuaternionNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/glm/gtx/quaternion.hpp>


using namespace noggit::ui::tools::NodeEditor::Nodes;

MatrixRotateQuaternionNode::MatrixRotateQuaternionNode()
: BaseNode()
{
  setName("Matrix :: RotateQuaternion");
  setCaption("Matrix :: RotateQuaternion");
  _validation_state = NodeValidationState::Valid;

  addPort<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addPort<QuaternionData>(PortType::In, "Quaternion", true);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixRotateQuaternionNode::compute()
{
  glm::mat4 matrix = static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get())->value();
  glm::quat quat = static_cast<QuaternionData*>(_in_ports[1].in_value.lock().get())->value();

  glm::mat4 rot_matrix = glm::toMat4(quat);

  _out_ports[0].out_value = std::make_shared<Matrix4x4Data>(matrix * rot_matrix);
  _node->onDataUpdated(0);
}

NodeValidationState MatrixRotateQuaternionNode::validate()
{
  if (!static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate matrix input.");
    return _validation_state;
  }

  if (!static_cast<QuaternionData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate quaternion input.");
    return _validation_state;
  }

  return _validation_state;
}
