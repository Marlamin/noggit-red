#ifndef NOGGIT_MATHNODE_HPP
#define NOGGIT_MATHNODE_HPP

#include "noggit/Red/NodeEditor/Nodes/BaseNode.hpp"

#include <unordered_map>
#include <QComboBox>
#include <array>
#include <string_view>
#include <functional>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;
using namespace std::literals;

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
            NodeValidationState validate() override;


        public Q_SLOTS:
            void inputConnectionCreated(const Connection& connection) override;
            void inputConnectionDeleted(const Connection& connection) override;

        protected:

            template <typename T>
            T commonCast(NodeData* data, int type_id);

            template <typename T>
            void handleOperation(T first, T second);

            QComboBox* _operation;

            static constexpr std::array<std::pair<std::string_view, int>, 4> _types {std::pair{"int"sv, 0},
                                                                                     std::pair{"uint"sv, 1},
                                                                                     std::pair{"decimal"sv, 2},
                                                                                     std::pair{"string"sv, 3}};

            int _first_type;
            int _second_type;
            int _result_type;

        };

    }

}


#endif //NOGGIT_MATHNODE_HPP
