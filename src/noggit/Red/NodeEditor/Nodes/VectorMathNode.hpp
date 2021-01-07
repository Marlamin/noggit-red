// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_VECTORMATHNODE_HPP
#define NOGGIT_VECTORMATHNODE_HPP

#include "BaseNode.hpp"
#include "Data/GenericData.hpp"
#include <QComboBox>

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
      class VectorMathNodeBase : public BaseNode
      {

      public:
        VectorMathNodeBase();
        void compute() override;
        QJsonObject save() const override;
        void restore(QJsonObject const& json_obj) override;

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
