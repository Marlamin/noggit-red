// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_MATRIXROTATEQUATERNIONNODE_HPP
#define NOGGIT_MATRIXROTATEQUATERNIONNODE_HPP

#include "noggit/Red/NodeEditor/Nodes/BaseNode.hpp"

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
        class MatrixRotateQuaternionNode : public BaseNode
        {
        Q_OBJECT

        public:
            MatrixRotateQuaternionNode();
            NodeValidationState validate() override;
            void compute() override;

        };

    }

}

#endif //NOGGIT_MATRIXROTATEQUATERNIONNODE_HPP
