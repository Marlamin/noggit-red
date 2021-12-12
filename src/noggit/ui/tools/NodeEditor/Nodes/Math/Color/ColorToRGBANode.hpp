// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_COLORTORGBANODE_HPP
#define NOGGIT_COLORTORGBANODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class ColorToRGBANode : public BaseNode
        {
        Q_OBJECT

        public:
            ColorToRGBANode();
            void compute() override;
            NodeValidationState validate() override;
        };

    }

}

#endif //NOGGIT_COLORTORGBANODE_HPP
