// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_VECTORSCALARMATHNODE_HPP
#define NOGGIT_VECTORSCALARMATHNODE_HPP

#include "BaseNode.hpp"
#include <QComboBox>

#include "Data/GenericData.hpp"

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
        VectorScalarMathNodeBase();
        void compute() override;
        QJsonObject save() const override;
        void restore(QJsonObject const& json_obj) override;

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
