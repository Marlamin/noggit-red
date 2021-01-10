// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_VECTOR4DTOXYZWNODE_HPP
#define NOGGIT_VECTOR4DTOXYZWNODE_HPP

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
        class Vector4DToXYZWNode : public BaseNode
        {
        Q_OBJECT

        public:
            Vector4DToXYZWNode();
            NodeValidationState validate() override;
            void compute() override;
        };

    }

}

#endif //NOGGIT_VECTOR4DTOXYZWNODE_HPP
