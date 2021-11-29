// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEGETPIXELNODE_HPP
#define NOGGIT_IMAGEGETPIXELNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>

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
        class ImageGetPixelNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            ImageGetPixelNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:

        };

    }

}

#endif //NOGGIT_IMAGEGETPIXELNODE_HPP
