// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETWATERTYPE_HPP
#define NOGGIT_GETWATERTYPE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextNodeBase.hpp>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace noggit
{
    namespace ui::tools::NodeEditor::Nodes
    {
        class GetWaterTypeNode : public ContextNodeBase
        {
        Q_OBJECT

        public:
            GetWaterTypeNode();
            void compute() override;
        };

    }

}

#endif //NOGGIT_GETWATERTYPE_HPP
