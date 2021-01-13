// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISECYLINDERSNODE_HPP
#define NOGGIT_NOISECYLINDERSNODE_HPP

#include "NoiseGeneratorBase.hpp"

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
        class NoiseCylindersNode : public NoiseGeneratorBase
        {
        Q_OBJECT

        public:
            NoiseCylindersNode();
            void compute() override;
        };

    }

}

#endif //NOGGIT_NOISECYLINDERSNODE_HPP
