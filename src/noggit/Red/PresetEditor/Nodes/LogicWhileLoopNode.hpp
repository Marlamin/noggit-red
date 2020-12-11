// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICWHILELOOPNODE_HPP
#define NOGGIT_LOGICWHILELOOPNODE_HPP

#include "LogicNodeBase.hpp"

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
        class LogicWhileLoopNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicWhileLoopNode();
            void compute() override;
            NodeValidationState validate() override;
        };

    }

}

#endif //NOGGIT_LOGICWHILELOOPNODE_HPP
