// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGECREATENODE_HPP
#define NOGGIT_IMAGECREATENODE_HPP

#include <noggit/Red/NodeEditor/Nodes/LogicNodeBase.hpp>
#include <QComboBox>

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
        class ImageCreateNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            ImageCreateNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QComboBox* _image_format;

        };

    }

}

#endif //NOGGIT_IMAGECREATENODE_HPP
