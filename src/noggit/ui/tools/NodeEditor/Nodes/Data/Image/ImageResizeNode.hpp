// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGERESIZENODE_HPP
#define NOGGIT_IMAGERESIZENODE_HPP

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
        class ImageResizeNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            ImageResizeNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
          QComboBox* _aspect_ratio_mode;
          QComboBox* _mode;
        };

    }

}

#endif //NOGGIT_IMAGERESIZENODE_HPP
