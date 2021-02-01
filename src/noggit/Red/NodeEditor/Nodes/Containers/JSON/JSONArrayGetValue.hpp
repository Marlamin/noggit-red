// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_JSONARRAYGETVALUE_HPP
#define NOGGIT_JSONARRAYGETVALUE_HPP

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
        class JSONArrayGetValueNode : public BaseNode
        {
        Q_OBJECT

        public:
            JSONArrayGetValueNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        };

    }

}

#endif //NOGGIT_JSONARRAYGETVALUE_HPP
