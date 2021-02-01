// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETJSONVALUE_HPP
#define NOGGIT_GETJSONVALUE_HPP

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
        class GetJSONValueNode : public BaseNode
        {
        Q_OBJECT

        public:
            GetJSONValueNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;
            NodeValidationState validate() override;
        };

    }

}

#endif //NOGGIT_GETJSONVALUE_HPP
