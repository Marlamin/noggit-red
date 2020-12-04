#ifndef NOGGIT_LOGICIFNODE_HPP
#define NOGGIT_LOGICIFNODE_HPP

#include "BaseNode.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::PresetEditor::Nodes
    {
        class LogicIfNode : public BaseNode
        {
        Q_OBJECT

        public:
            LogicIfNode();
            void compute() override;
        };

    }

}

#endif //NOGGIT_LOGICIFNODE_HPP
