// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_XYZTOVECTOR3DNODE_HPP
#define NOGGIT_XYZTOVECTOR3DNODE_HPP

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
        class XYZtoVector3DNode : public BaseNode
        {
        Q_OBJECT

        public:
            XYZtoVector3DNode();
            void compute() override;
            NodeValidationState validate() override;

        };

    }

}

#endif //NOGGIT_XYZTOVECTOR3DNODE_HPP
