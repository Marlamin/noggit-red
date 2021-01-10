// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETVARIABLELAZYNODE_HPP
#define NOGGIT_GETVARIABLELAZYNODE_HPP

#include "noggit/Red/NodeEditor/Nodes/BaseNode.hpp"
#include <external/tsl/robin_map.h>

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
        using VariableMap = tsl::robin_map<std::string, std::pair<std::string, std::shared_ptr<NodeData>>>;

        class GetVariableLazyNodeBase : public BaseNode
        {
        Q_OBJECT

        public:
            GetVariableLazyNodeBase();

            void compute() override;

            QJsonObject save() const override;

            void restore(QJsonObject const &json_obj) override;

        public Q_SLOTS:

            void outputConnectionCreated(const Connection &connection) override;

            void outputConnectionDeleted(const Connection &connection) override;

        protected:
            virtual VariableMap *getVariableMap() = 0;
        };

        // Scene scope

        class GetVariableLazyNode : public GetVariableLazyNodeBase
        {
        Q_OBJECT

        public:
            GetVariableLazyNode();

        protected:
            VariableMap *getVariableMap() override;
        };

        // Context scope

        class GetContextVariableLazyNode : public GetVariableLazyNodeBase
        {
        Q_OBJECT

        public:
            GetContextVariableLazyNode();

        protected:
            VariableMap *getVariableMap() override;
        };

    }

}

#endif //NOGGIT_GETVARIABLELAZYNODE_HPP
