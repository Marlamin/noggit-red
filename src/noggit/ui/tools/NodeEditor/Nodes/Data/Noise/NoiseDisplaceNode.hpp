// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEDISPLACENODE_HPP
#define NOGGIT_NOISEDISPLACENODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>
#include <external/libnoise/src/noise/noise.h>

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
        class NoiseDisplaceNode : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseDisplaceNode();
            void compute() override;
            NodeValidationState validate() override;

        private:
            noise::module::Displace _module;
        };

    }

}

#endif //NOGGIT_NOISEDISPLACENODE_HPP
