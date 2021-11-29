// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_JSONARRAYPUSH_HPP
#define NOGGIT_JSONARRAYPUSH_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>
#include <QComboBox>

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
        class JSONArrayPushNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            JSONArrayPushNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
          QComboBox* _operation;
        };

    }

}

#endif //NOGGIT_JSONARRAYPUSH_HPP
