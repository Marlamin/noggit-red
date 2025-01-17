#ifndef NOGGIT_CONDITIONNODE_HPP
#define NOGGIT_CONDITIONNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>

#include <QComboBox>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class QDoubleSpinBox;
class QComboBox;

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class ConditionNode : public BaseNode
        {
        Q_OBJECT

        public:
            ConditionNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QDoubleSpinBox* _first;
            QDoubleSpinBox* _second;
            QComboBox* _operation;


        };

    }

}

#endif //NOGGIT_CONDITIONNODE_HPP
