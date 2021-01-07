// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_MATRIXMATHNODE_HPP
#define NOGGIT_MATRIXMATHNODE_HPP

#include "BaseNode.hpp"
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
        class MatrixMathNode : public BaseNode
        {
        Q_OBJECT

        public:
            MatrixMathNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
          QComboBox* _operation;

        };

    }

}

#endif //NOGGIT_MATRIXMATHNODE_HPP
