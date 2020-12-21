#ifndef NOGGIT_LOGICIFNODE_HPP
#define NOGGIT_LOGICIFNODE_HPP

#include "LogicNodeBase.hpp"

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
        class LogicIfNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicIfNode();
            void compute() override;
            NodeValidationState validate() override;
        };

    }

}

#endif //NOGGIT_LOGICIFNODE_HPP
