// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISECONSTVALUENODE_HPP
#define NOGGIT_NOISECONSTVALUENODE_HPP

#include "NoiseGeneratorBase.hpp"
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
        class NoiseConstValueNode : public NoiseGeneratorBase
        {
        Q_OBJECT

        public:
            NoiseConstValueNode();
            void compute() override;

        private:
            noise::module::Const _module;
        };

    }

}

#endif //NOGGIT_NOISECONSTVALUENODE_HPP
