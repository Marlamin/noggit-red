// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISERIDGEDMULTINODE_HPP
#define NOGGIT_NOISERIDGEDMULTINODE_HPP

#include "NoiseGeneratorBase.hpp"
#include <external/libnoise/src/noise/noise.h>
#include <QComboBox>

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
        class NoiseRidgedMultiNode : public NoiseGeneratorBase
        {
        Q_OBJECT

        public:
            NoiseRidgedMultiNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QComboBox* _quality;
            noise::module::RidgedMulti _module;
        };

    }

}

#endif //NOGGIT_NOISERIDGEDMULTINODE_HPP
