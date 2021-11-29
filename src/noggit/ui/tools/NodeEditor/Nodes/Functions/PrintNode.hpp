#ifndef NOGGIT_PRINTNODE_HPP
#define NOGGIT_PRINTNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"
#include <QLineEdit>

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
        class PrintNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            PrintNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QLineEdit* _text;

        };

    }

}

#endif //NOGGIT_PRINTNODE_HPP
