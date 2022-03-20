// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETVARIABLENODE_HPP
#define NOGGIT_GETVARIABLENODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>
#include <external/tsl/robin_map.h>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        using VariableMap = tsl::robin_map<std::string, std::pair<std::string, std::shared_ptr<NodeData>>>;

        class GetVariableNodeBase : public LogicNodeBase
        {
        Q_OBJECT

        public:
            GetVariableNodeBase();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        public Q_SLOTS:
            void outputConnectionCreated(const Connection& connection) override;
            void outputConnectionDeleted(const Connection& connection) override;

        protected:
            virtual VariableMap* getVariableMap() = 0;
        };

        // Scene scope

        class GetVariableNode : public GetVariableNodeBase
        {
            Q_OBJECT

        public:
            GetVariableNode();

        protected:
            VariableMap* getVariableMap() override;
        };

        // Context scope

        class GetContextVariableNode : public GetVariableNodeBase
        {
        Q_OBJECT

        public:
            GetContextVariableNode();

        protected:
            VariableMap* getVariableMap() override;
        };

    }

}

#endif //NOGGIT_GETVARIABLENODE_HPP
