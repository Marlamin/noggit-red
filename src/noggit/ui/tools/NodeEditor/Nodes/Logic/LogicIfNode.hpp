#ifndef NOGGIT_LOGICIFNODE_HPP
#define NOGGIT_LOGICIFNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"

using QtNodes::NodeValidationState;

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
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
