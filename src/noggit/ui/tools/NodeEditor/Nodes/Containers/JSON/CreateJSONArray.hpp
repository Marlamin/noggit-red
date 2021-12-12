// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CREATEJSONARRAY_HPP
#define NOGGIT_CREATEJSONARRAY_HPP

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
