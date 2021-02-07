// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEABSNODE_HPP
#define NOGGIT_NOISEABSNODE_HPP

#include <noggit/Red/NodeEditor/Nodes/BaseNode.hpp>
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
        class NoiseAbsNode : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseAbsNode();
            void compute() override;
            NodeValidationState validate() override;

        private:
            noise::module::Abs _module;
        };

    }

}

#endif //NOGGIT_NOISEABSNODE_HPP
