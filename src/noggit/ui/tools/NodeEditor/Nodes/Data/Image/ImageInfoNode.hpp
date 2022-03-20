// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEINFONODE_HPP
#define NOGGIT_IMAGEINFONODE_HPP

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
        class ImageInfoNode : public BaseNode
        {
        Q_OBJECT

        public:
            ImageInfoNode();
            NodeValidationState validate() override;
            void compute() override;
        };

    }

}

#endif //NOGGIT_IMAGEINFONODE_HPP
