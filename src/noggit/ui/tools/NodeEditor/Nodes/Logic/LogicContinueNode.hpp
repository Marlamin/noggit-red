// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICCONTINUENODE_HPP
#define NOGGIT_LOGICCONTINUENODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class LogicContinueNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicContinueNode();
            void compute() override;
            NodeValidationState validate() override;
            bool doContinue() { return _do_continue; };
            void setDoContinue(bool state) { _do_continue = state; };

        private:
            bool _do_continue = false;

        };

    }

}

#endif //NOGGIT_LOGICCONTINUENODE_HPP
