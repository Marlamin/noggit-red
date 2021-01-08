// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOADIMAGENODE_HPP
#define NOGGIT_LOADIMAGENODE_HPP

#include "LogicNodeBase.hpp"

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
        class LoadImageNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LoadImageNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:

        };

    }

}

#endif //NOGGIT_LOADIMAGENODE_HPP
