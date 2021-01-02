// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_SETVARIABLENODE_HPP
#define NOGGIT_SETVARIABLENODE_HPP

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
        class SetVariableNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            SetVariableNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        public Q_SLOTS:
            void inputConnectionCreated(const Connection& connection) override;
            void inputConnectionDeleted(const Connection& connection) override;

        };

    }

}

#endif //NOGGIT_SETVARIABLENODE_HPP
