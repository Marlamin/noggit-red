// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CREATEJSONARRAY_HPP
#define NOGGIT_CREATEJSONARRAY_HPP

#include <noggit/Red/NodeEditor/Nodes/LogicNodeBase.hpp>

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
        class CreateJSONArrayNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            CreateJSONArrayNode();
            void compute() override;
        };

    }

}
#endif //NOGGIT_CREATEJSONARRAY_HPP
