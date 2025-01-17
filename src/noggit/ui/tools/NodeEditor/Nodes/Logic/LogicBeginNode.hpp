#ifndef NOGGIT_LOGICBEGINNODE_HPP
#define NOGGIT_LOGICBEGINNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"
#include <vector>

using QtNodes::PortType;
using QtNodes::Node;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
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
            void reset();

        public Q_SLOTS:
            void outputConnectionCreated(Connection const& connection) override;
            void outputConnectionDeleted(Connection const& connection) override;
            void portDoubleClicked(PortType port_type, PortIndex port_index) override;

        private:
            std::vector<QWidget*> _default_widgets;
        };
    }
}

#endif //NOGGIT_LOGICBEGINNODE_HPP
