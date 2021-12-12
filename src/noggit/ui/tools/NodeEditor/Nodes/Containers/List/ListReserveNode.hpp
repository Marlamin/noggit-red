// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LISTRESERVENODE_HPP
#define NOGGIT_LISTRESERVENODE_HPP

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
        class ListReserveNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            ListReserveNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        };

    }

}

#endif //NOGGIT_LISTRESERVENODE_HPP
