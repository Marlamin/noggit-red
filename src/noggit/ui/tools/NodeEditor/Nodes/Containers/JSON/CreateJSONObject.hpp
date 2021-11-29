// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CREATEJSONOBJECT_HPP
#define NOGGIT_CREATEJSONOBJECT_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>

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
        class CreateJSONObjectNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            CreateJSONObjectNode();
            void compute() override;
        };

    }

}

#endif //NOGGIT_CREATEJSONOBJECT_HPP
