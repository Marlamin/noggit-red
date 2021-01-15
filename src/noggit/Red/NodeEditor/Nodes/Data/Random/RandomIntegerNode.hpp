// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_RANDOMINTEGERNODE_HPP
#define NOGGIT_RANDOMINTEGERNODE_HPP

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
        class RandomIntegerNode : public BaseNode
        {
        Q_OBJECT

        public:
            RandomIntegerNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;
        };

    }

}

#endif //NOGGIT_RANDOMINTEGERNODE_HPP
