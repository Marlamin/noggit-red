// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_STRINGEQUAL_HPP
#define NOGGIT_STRINGEQUAL_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>

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
        class StringEqualNode : public BaseNode
        {
        Q_OBJECT

        public:
            StringEqualNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        };

    }

}

#endif //NOGGIT_STRINGEQUAL_HPP
