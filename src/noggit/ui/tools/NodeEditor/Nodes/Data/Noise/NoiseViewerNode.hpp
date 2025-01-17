// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEVIEWERNODE_HPP
#define NOGGIT_NOISEVIEWERNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class QLabel;
class QPushButton;

namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class NoiseViewerNode : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseViewerNode();
            NodeValidationState validate() override;
            void compute() override;

        private:
          QPushButton* _update_btn;
          QLabel* _image;
        };
    }
}

#endif //NOGGIT_NOISEVIEWERNODE_HPP
