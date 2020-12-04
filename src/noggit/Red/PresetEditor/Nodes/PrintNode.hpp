#ifndef NOGGIT_PRINTNODE_HPP
#define NOGGIT_PRINTNODE_HPP

#include "BaseNode.hpp"
#include <QLineEdit>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
    namespace Red::PresetEditor::Nodes
    {
        class PrintNode : public BaseNode
        {
        Q_OBJECT

        public:
            PrintNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QLineEdit* _text;

        };

    }

}

#endif //NOGGIT_PRINTNODE_HPP
