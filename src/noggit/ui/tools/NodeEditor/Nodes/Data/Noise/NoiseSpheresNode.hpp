// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISESPHERESNODE_HPP
#define NOGGIT_NOISESPHERESNODE_HPP

#include "NoiseGeneratorBase.hpp"
#include <external/libnoise/src/noise/noise.h>

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
        class NoiseSpheresNode : public NoiseGeneratorBase
        {
        Q_OBJECT

        public:
            NoiseSpheresNode();
            void compute() override;

        private:
            noise::module::Spheres _module;
        };

    }

}

#endif //NOGGIT_NOISESPHERESNODE_HPP
