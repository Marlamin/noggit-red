// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_VECTORSCALARMATHNODE_HPP
#define NOGGIT_VECTORSCALARMATHNODE_HPP

#include "noggit/Red/NodeEditor/Nodes/BaseNode.hpp"
#include <QComboBox>

#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {
      template <typename T, typename T1>
      class VectorScalarMathNodeBase : public BaseNode
      {

      public:
        VectorScalarMathNodeBase()
        {
          setCaption("Dot");
          _validation_state = NodeValidationState::Valid;

          _operation = new QComboBox(&_embedded_widget);
          _operation->addItems({"Dot", "Distance"});

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

          addPort<DecimalData>(PortType::Out, "Decimal", true);
        };

        void compute() override
        {
          auto vector_1_ptr = static_cast<T*>(_in_ports[0].in_value.lock().get());
          T1 vector_1 = vector_1_ptr ? vector_1_ptr->value()
                                     : static_cast<T*>(_in_ports[0].data_type->default_widget_data(_in_ports[0].default_widget).get())->value();

          auto vector_2_ptr = static_cast<T*>(_in_ports[1].in_value.lock().get());
          T1 vector_2 = vector_2_ptr ? vector_2_ptr->value()
                                     : static_cast<T*>(_in_ports[1].data_type->default_widget_data(_in_ports[1].default_widget).get())->value();

          double result;
          switch (_operation->currentIndex())
          {
            case 0: // Dot
              result = glm::dot(vector_1, vector_2);
              break;
            case 1: // Distance
              result = (vector_1 - vector_2).length();
              break;
          }

          _out_ports[0].out_value = std::make_shared<DecimalData>(result);
          Q_EMIT dataUpdated(0);
        };

        QJsonObject save() const override
        {
          QJsonObject json_obj = BaseNode::save();

          json_obj["operation"] = _operation->currentIndex();
          _in_ports[0].data_type->to_json(_in_ports[0].default_widget, json_obj, "vector_1");
          _in_ports[1].data_type->to_json(_in_ports[1].default_widget, json_obj, "vector_2");

          return json_obj;

        };

        void restore(QJsonObject const& json_obj) override
        {
          _in_ports[0].data_type->from_json(_in_ports[0].default_widget, json_obj, "vector_1");
          _in_ports[1].data_type->from_json(_in_ports[1].default_widget, json_obj, "vector_2");
          _operation->setCurrentIndex(json_obj["operation"].toInt());
          BaseNode::restore(json_obj);
        };

      private:
        QComboBox* _operation;
      };

      class Vector3DScalarMathNode : public VectorScalarMathNodeBase<Vector3DData, glm::vec3>
      {
      public:
          Vector3DScalarMathNode();
      };

      class Vector2DScalarMathNode : public VectorScalarMathNodeBase<Vector2DData, glm::vec2>
      {
      public:
          Vector2DScalarMathNode();
      };

      class Vector4DScalarMathNode : public VectorScalarMathNodeBase<Vector4DData, glm::vec4>
      {
      public:
          Vector4DScalarMathNode();
      };


    }

}

#endif //NOGGIT_VECTORSCALARMATHNODE_HPP
