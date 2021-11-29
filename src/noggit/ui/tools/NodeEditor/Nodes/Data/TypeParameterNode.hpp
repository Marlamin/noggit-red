// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TYPEPARAMETERNODE_HPP
#define NOGGIT_TYPEPARAMETERNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp"

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
        class TypeParameterNode : public BaseNode
        {
        Q_OBJECT

        public:
            TypeParameterNode();
            void compute() override;
            NodeValidationState validate() override;

        private:

        };

    }

}

#endif //NOGGIT_TYPEPARAMETERNODE_HPP
