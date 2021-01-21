// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_REWIRINGPOINTNODE_HPP
#define NOGGIT_REWIRINGPOINTNODE_HPP

#include <noggit/Red/NodeEditor/Nodes/BaseNode.hpp>

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
        class RewiringPointNode : public BaseNode
        {
        Q_OBJECT

        public:
            RewiringPointNode();
            void compute() override;
            QJsonObject save() const override;
            NodeValidationState validate() override;
            void restore(QJsonObject const& json_obj) override;

            bool captionVisible() const override { return false; };

        public Q_SLOTS:

            void inputConnectionCreated(Connection const&) override;

            void inputConnectionDeleted(Connection const&) override;
        };

    }

}

#endif //NOGGIT_REWIRINGPOINTNODE_HPP
