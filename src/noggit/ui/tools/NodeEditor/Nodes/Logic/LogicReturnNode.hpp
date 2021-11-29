// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICRETURNNODE_HPP
#define NOGGIT_LOGICRETURNNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"
#include <external/NodeEditor/include/nodes/Node>

using QtNodes::PortType;
using QtNodes::Node;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace ui::tools::NodeEditor::Nodes
    {
        class LogicReturnNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicReturnNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;
            void restorePostConnection(const QJsonObject& json_obj) override;
            std::vector<InNodePort>* getOutSignature() { return &_in_ports; };

        public Q_SLOTS:
            void inputConnectionCreated(Connection const& connection) override;
            void inputConnectionDeleted(Connection const& connection) override;
            void portDoubleClicked(PortType port_type, PortIndex port_index) override;
        };

    }

}

#endif //NOGGIT_LOGICRETURNNODE_HPP
