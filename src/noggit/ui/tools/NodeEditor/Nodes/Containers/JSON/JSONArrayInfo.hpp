// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_JSONARRAYINFO_HPP
#define NOGGIT_JSONARRAYINFO_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>

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
        class JSONArrayInfoNode : public BaseNode
        {
        Q_OBJECT

        public:
            JSONArrayInfoNode();
            void compute() override;
            NodeValidationState validate() override;

        };

    }

}

#endif //NOGGIT_JSONARRAYINFO_HPP
