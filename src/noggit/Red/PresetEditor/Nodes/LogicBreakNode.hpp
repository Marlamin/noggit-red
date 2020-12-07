// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICBREAKNODE_HPP
#define NOGGIT_LOGICBREAKNODE_HPP

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
        class LogicBreakNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicBreakNode();
            void compute() override;
            NodeValidationState validate() override;
            bool doBreak() { return _do_break; };
            bool setDoBreak(bool state) { _do_break = state; };

        private:
            bool _do_break = false;

        };

    }

}

#endif //NOGGIT_LOGICBREAKNODE_HPP
