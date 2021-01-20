#ifndef NOGGIT_MATHNODE_HPP
#define NOGGIT_MATHNODE_HPP

#include "noggit/Red/NodeEditor/Nodes/BaseNode.hpp"

#include <unordered_map>
#include <QComboBox>

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
        class MathNode : public BaseNode
        {
        Q_OBJECT

        public:
            MathNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        protected:
            QComboBox* _operation;

        };

    }

}


#endif //NOGGIT_MATHNODE_HPP
