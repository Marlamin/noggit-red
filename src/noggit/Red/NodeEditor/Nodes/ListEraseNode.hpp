// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LISTERASENODE_HPP
#define NOGGIT_LISTERASENODE_HPP

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
        class ListEraseNode : public LogicNodeBase
        {
            Q_OBJECT

        public:
            ListEraseNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        };

    }

}


#endif //NOGGIT_LISTERASENODE_HPP
