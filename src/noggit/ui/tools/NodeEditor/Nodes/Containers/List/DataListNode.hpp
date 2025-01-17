// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_DATALISTNODE_HPP
#define NOGGIT_DATALISTNODE_HPP

#include "noggit/ui/tools/NodeEditor/Nodes/Containers/List/ListNodeBase.hpp"

#include <vector>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class QComboBox;

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class DataListNode : public ListNodeBase
        {
        Q_OBJECT

        public:
            DataListNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QComboBox* _type;

            std::vector<std::shared_ptr<NodeData>> _data;
        };
    }
}

#endif //NOGGIT_DATALISTNODE_HPP
