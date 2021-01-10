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
            NodeValidationState validate() override;
            void restore(QJsonObject const& json_obj) override;

        public Q_SLOTS:
          void inputConnectionCreated(const Connection& connection) override;
          void inputConnectionDeleted(const Connection& connection) override;

        protected:
            QComboBox* _operation;
            std::unordered_map<std::string, int> _type_map = {{"int", 0},
                                                              {"uint", 1},
                                                              {"double", 2},
                                                              {"string", 3}};

            template <typename T>
            T commonCast(NodeData* data);

            template <typename T>
            void handleOperation(T first, T second);

        };

    }

}


#endif //NOGGIT_MATHNODE_HPP
