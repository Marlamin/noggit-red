// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETVARIABLENODE_HPP
#define NOGGIT_GETVARIABLENODE_HPP

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
        class GetVariableNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            GetVariableNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        public Q_SLOTS:
            void outputConnectionCreated(const Connection& connection) override;
            void outputConnectionDeleted(const Connection& connection) override;

        private:
        };

    }

}

#endif //NOGGIT_GETVARIABLENODE_HPP
