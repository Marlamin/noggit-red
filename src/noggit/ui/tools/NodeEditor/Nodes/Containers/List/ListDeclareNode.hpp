// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LISTDECLARENODE_HPP
#define NOGGIT_LISTDECLARENODE_HPP

#include "ListNodeBase.hpp"

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
        class ListDeclareNode : public ListNodeBase
        {
        Q_OBJECT

        public:
            ListDeclareNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;
            bool isLogicNode() override { return false; };

        private:
            QComboBox* _type;
        };
    }
}

#endif //NOGGIT_LISTDECLARENODE_HPP
