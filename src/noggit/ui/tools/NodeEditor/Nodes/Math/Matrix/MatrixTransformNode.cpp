// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MatrixTransformNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/glm/gtx/transform.hpp>
#include <external/glm/gtx/euler_angles.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

MatrixTransformNode::MatrixTransformNode()
: BaseNode()
{
  setName("Matrix :: Transform");
  setCaption("Matrix :: Transform");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Translate", "Rotate Euler", "Scale"});

  addWidgetTop(_operation);

  addPortDefault<Matrix4x4Data>(PortType::In, "Matrix4x4", true);
  addPortDefault<Vector3DData>(PortType::In, "Vector3D", true);
  addPort<Matrix4x4Data>(PortType::Out, "Matrix4x4", true);
}

void MatrixTransformNode::compute()
{
  glm::mat4 matrix = defaultPortData<Matrix4x4Data>(PortType::In, 0)->value();

  auto transform_vec_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& transform_vec = transform_vec_data->value();

  switch (_operation->currentIndex())
  {
    case 0: // Translate
      matrix = glm::translate(matrix, transform_vec);
      break;

    case 1: // Rotate Euler
      matrix = matrix * glm::eulerAngleYZX(glm::radians(transform_vec.z),
                                           glm::radians(transform_vec.y),
                                           glm::radians(transform_vec.x));
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
  defaultWidgetToJson(PortType::In, 1, json_obj, "transform_vector");
  return json_obj;
}

void MatrixTransformNode::restore(const QJsonObject& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());
  BaseNode::restore(json_obj);
  defaultWidgetFromJson(PortType::In, 1, json_obj, "transform_vector");
}

NodeValidationState MatrixTransformNode::validate()
{
  if (!static_cast<Matrix4x4Data*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate matrix input.");
    return _validation_state;
  }

  return _validation_state;
}
