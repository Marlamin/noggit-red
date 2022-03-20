// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOADJSONOBJECT_HPP
#define NOGGIT_LOADJSONOBJECT_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>

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
        class LoadJSONObjectNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LoadJSONObjectNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;
        };

    }

}

#endif //NOGGIT_LOADJSONOBJECT_HPP
