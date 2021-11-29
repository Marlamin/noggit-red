// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_SETVARIABLENODE_HPP
#define NOGGIT_SETVARIABLENODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp"
#include <external/tsl/robin_map.h>

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
        using VariableMap = tsl::robin_map<std::string, std::pair<std::string, std::shared_ptr<NodeData>>>;

        class SetVariableNodeBase : public LogicNodeBase
        {
        Q_OBJECT

        public:
            SetVariableNodeBase();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        public Q_SLOTS:
            void inputConnectionCreated(const Connection& connection) override;
            void inputConnectionDeleted(const Connection& connection) override;

        protected:
            virtual VariableMap* getVariableMap() = 0;

        };

        // Scene scope

        class SetVariableNode : public SetVariableNodeBase
        {
        Q_OBJECT

        public:
            SetVariableNode();

        protected:
            VariableMap* getVariableMap() override;
        };

        // Context scope

        class SetContextVariableNode : public SetVariableNodeBase
        {
        Q_OBJECT

        public:
            SetContextVariableNode();

        protected:
            VariableMap* getVariableMap() override;
        };

    }

}

#endif //NOGGIT_SETVARIABLENODE_HPP
