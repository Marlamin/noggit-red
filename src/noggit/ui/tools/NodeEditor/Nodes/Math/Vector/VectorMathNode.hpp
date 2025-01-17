// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_VECTORMATHNODE_HPP
#define NOGGIT_VECTORMATHNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp"
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/NodeEditor/include/nodes/Node>

#include <QComboBox>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
      template <typename T, typename T1>
      class VectorMathNodeBase : public BaseNode
      {

      public:
        VectorMathNodeBase()
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
        };

        void compute() override
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
          _node->onDataUpdated(0);
        };

        QJsonObject save() const override
        {
          QJsonObject json_obj = BaseNode::save();

          json_obj["operation"] = _operation->currentIndex();
          _in_ports[0].data_type->to_json(_in_ports[0].default_widget, json_obj, "vector_1");
          _in_ports[1].data_type->to_json(_in_ports[1].default_widget, json_obj, "vector_2");

          return json_obj;
        }

        void restore(QJsonObject const& json_obj) override
        {
          _in_ports[0].data_type->from_json(_in_ports[0].default_widget, json_obj, "vector_1");
          _in_ports[1].data_type->from_json(_in_ports[1].default_widget, json_obj, "vector_2");
          _operation->setCurrentIndex(json_obj["operation"].toInt());
          BaseNode::restore(json_obj);
        };

      protected:
        QComboBox* _operation;
      };

      class Vector3DMathNode : public VectorMathNodeBase<Vector3DData, glm::vec3>
      {

      public:
        Vector3DMathNode();
      };

      class Vector2DMathNode : public VectorMathNodeBase<Vector2DData, glm::vec2>
      {

      public:
        Vector2DMathNode();
      };

      class Vector4DMathNode : public VectorMathNodeBase<Vector4DData, glm::vec4>
      {

      public:
        Vector4DMathNode();
      };

    }

}

#endif //NOGGIT_VECTORMATHNODE_HPP
