// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_VECTOR2DTOXYNODE_HPP
#define NOGGIT_VECTOR2DTOXYNODE_HPP

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
        class Vector2DToXYNode : public BaseNode
        {
        Q_OBJECT

        public:
            Vector2DToXYNode();
            NodeValidationState validate() override;
            void compute() override;
        };

    }

}

#endif //NOGGIT_VECTOR2DTOXYNODE_HPP
