// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISECURVENODE_HPP
#define NOGGIT_NOISECURVENODE_HPP

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
        class NoiseCurveNode : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseCurveNode();
            NodeValidationState validate() override;
            void compute() override;

        private:
            noise::module::Curve _module;
        };

    }

}

#endif //NOGGIT_NOISECURVENODE_HPP
