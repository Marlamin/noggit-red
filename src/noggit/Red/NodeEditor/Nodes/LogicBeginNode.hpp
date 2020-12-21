#ifndef NOGGIT_LOGICBEGINNODE_HPP
#define NOGGIT_LOGICBEGINNODE_HPP

#include "LogicNodeBase.hpp"
#include <external/NodeEditor/include/nodes/Node>
#include <vector>

using QtNodes::PortType;
using QtNodes::Node;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::NodeEditor::Nodes
    {
        class LogicBeginNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicBeginNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;
            void restorePostConnection(const QJsonObject& json_obj) override;
            NodeValidationState validate() override;
            std::vector<OutNodePort>* getInSignature() { return &_out_ports; };

        public Q_SLOTS:
            void outputConnectionCreated(Connection const& connection) override;
            void outputConnectionDeleted(Connection const& connection) override;

        private:
            std::vector<QWidget*> _default_widgets;

        };

    }

}

#endif //NOGGIT_LOGICBEGINNODE_HPP
