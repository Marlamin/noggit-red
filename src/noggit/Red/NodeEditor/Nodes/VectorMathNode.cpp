// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "VectorMathNode.hpp"

#include "BaseNode.inl"

using namespace noggit::Red::NodeEditor::Nodes;

template <typename T, typename T1>
VectorMathNodeBase<T, T1>::VectorMathNodeBase()
: BaseNode()
{
  setCaption("Add");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);

  if constexpr (std::is_same<T, glm::vec3>::value)
  {
    _operation->addItems({"Add", "Subtract", "Multiply", "Divide", "Cross"});
  }
  else
  {
    _operation->addItems({"Add", "Subtract", "Multiply", "Divide"});
  }


  QComboBox::connect(_operation, qOverload<int>(&QComboBox::currentIndexChanged)
      ,[this](int index)
     {
         setCaption(_operation->currentText());
     }
  );

  std::string type_name;

  if constexpr (std::is_same<T, glm::vec3>::value)
  {
    type_name = "Vector3D";
  }
  else if constexpr (std::is_same<T, glm::vec4>::value)
  {
    type_name = "Vector4D";
  }
  else
  {
    type_name = "Vector2D";
  }

  addWidgetTop(_operation);

  addPort<T>(PortType::In, type_name.c_str(), true);
  addDefaultWidget(_in_ports[0].data_type->default_widget(&_embedded_widget), PortType::In, 0);

  addPort<T>(PortType::In, type_name.c_str(), true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 1);

  addPort<T>(PortType::Out, type_name.c_str(), true);

}

template <typename T, typename T1>
void VectorMathNodeBase<T, T1>::compute()
{
  auto vector_1_ptr = static_cast<T*>(_in_ports[0].in_value.lock().get());
  T1 vector_1 = vector_1_ptr ? vector_1_ptr->value()
      : static_cast<T*>(_in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget).get())->value();

  auto vector_2_ptr = static_cast<T*>(_in_ports[1].in_value.lock().get());
  T1 vector_2 = vector_2_ptr ? vector_2_ptr->value()
      : static_cast<T*>(_in_ports[1].data_type->default_widget_data(_in_ports[1].default_widget).get())->value();

  T1 result;
  switch (_operation->currentIndex())
  {
    case 0: // Add
      result = vector_1 + vector_2;
      break;
    case 1: // Subtract
      result = vector_1 - vector_2;
      break;
    case 2: // Multiply
      result = vector_1 * vector_2;
      break;
    case 3: // Divide
      result = vector_1 / vector_2;
      break;
    case 4: // Cross
      if constexpr (std::is_same<T, glm::vec3>::value)
      {
        result = glm::cross(vector_1, vector_2);
      }
      break;
  }

  _out_ports[0].out_value = std::make_shared<T>(result);
  Q_EMIT dataUpdated(0);
}

template <typename T, typename T1>
QJsonObject VectorMathNodeBase<T, T1>::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();
  _in_ports[0].data_type->to_json(_in_ports[0].default_widget, json_obj, "vector_1");
  _in_ports[1].data_type->to_json(_in_ports[1].default_widget, json_obj, "vector_2");

  return json_obj;
}

template <typename T, typename T1>
void VectorMathNodeBase<T, T1>::restore(const QJsonObject& json_obj)
{
  _in_ports[0].data_type->from_json(_in_ports[0].default_widget, json_obj, "vector_1");
  _in_ports[1].data_type->from_json(_in_ports[1].default_widget, json_obj, "vector_2");
  _operation->setCurrentIndex(json_obj["operation"].toInt());
  BaseNode::restore(json_obj);
}


Vector3DMathNode::Vector3DMathNode()
{
  setName("Vector3DMathNode");
}

Vector2DMathNode::Vector2DMathNode()
{
  setName("Vector2DMathNode");
}

Vector4DMathNode::Vector4DMathNode()
{
  setName("Vector4DMathNode");
}
