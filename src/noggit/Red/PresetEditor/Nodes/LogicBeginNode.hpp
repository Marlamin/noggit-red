#ifndef NOGGIT_LOGICBEGINNODE_HPP
#define NOGGIT_LOGICBEGINNODE_HPP

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
        class LogicBeginNode : public BaseNode
        {
        Q_OBJECT

        public:
            LogicBeginNode();
            void compute() override;

        };

    }

}

#endif //NOGGIT_LOGICBEGINNODE_HPP
