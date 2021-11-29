// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICCHAINNODE_HPP
#define NOGGIT_LOGICCHAINNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace ui::tools::NodeEditor::Nodes
    {
        class LogicChainNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicChainNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;
            void restorePostConnection(const QJsonObject& json_obj) override;
            NodeValidationState validate() override;

        public Q_SLOTS:
            void outputConnectionCreated(Connection const& connection) override;
            void outputConnectionDeleted(Connection const& connection) override;

        private:

        };

    }

}

#endif //NOGGIT_LOGICCHAINNODE_HPP
