// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEVIEWERNODE_HPP
#define NOGGIT_NOISEVIEWERNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>
#include <QPushButton>
#include <QLabel>

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
