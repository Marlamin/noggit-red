// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEVORONOINODE_HPP
#define NOGGIT_NOISEVORONOINODE_HPP

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
        class NoiseVoronoiNode : public NoiseGeneratorBase
        {
        Q_OBJECT

        public:
            NoiseVoronoiNode();
            void compute() override;

        private:
            noise::module::Voronoi _module;
        };

    }

}

#endif //NOGGIT_NOISEVORONOINODE_HPP
