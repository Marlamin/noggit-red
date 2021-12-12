// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEBLENDNODE_HPP
#define NOGGIT_NOISEBLENDNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>
#include <external/libnoise/src/noise/noise.h>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace Noggit
{
    namespace Ui::Tools::NodeEditor::Nodes
    {
        class NoiseBlendNode : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseBlendNode();
            void compute() override;
            NodeValidationState validate() override;

        private:
            noise::module::Blend _module;
        };

    }

}

#endif //NOGGIT_NOISEBLENDNODE_HPP
