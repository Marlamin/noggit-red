// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_RANDOMSEEDNODE_HPP
#define NOGGIT_RANDOMSEEDNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>

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
        class RandomSeedNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            RandomSeedNode();
            void compute() override;

        };

    }

}
#endif //NOGGIT_RANDOMSEEDNODE_HPP
