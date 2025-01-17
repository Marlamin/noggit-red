// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEBLENDOPENGLNODE_HPP
#define NOGGIT_IMAGEBLENDOPENGLNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>

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
        class ImageBlendOpenGLNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            ImageBlendOpenGLNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QColor computeFactor(QColor const& source, QColor const& dest, int mode);
            QColor blendPixels(QColor const& source, QColor const& dest);

            QComboBox* _blend_func;
            QComboBox* _sfactor;
            QComboBox* _dfactor;
        };
    }
}

#endif //NOGGIT_IMAGEBLENDOPENGLNODE_HPP
