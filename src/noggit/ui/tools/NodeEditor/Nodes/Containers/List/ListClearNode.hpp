// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LISTCLEARNODE_HPP
#define NOGGIT_LISTCLEARNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"

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
        class ListClearNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            ListClearNode();
            void compute() override;
            NodeValidationState validate() override;

        };

    }

}

#endif //NOGGIT_LISTCLEARNODE_HPP
