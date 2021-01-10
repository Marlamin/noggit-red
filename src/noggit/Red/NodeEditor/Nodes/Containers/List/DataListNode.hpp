// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_DATALISTNODE_HPP
#define NOGGIT_DATALISTNODE_HPP

#include "noggit/Red/NodeEditor/Nodes/Containers/List/ListNodeBase.hpp"

#include <QComboBox>
#include <vector>
#include <external/tsl/robin_map.h>

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
