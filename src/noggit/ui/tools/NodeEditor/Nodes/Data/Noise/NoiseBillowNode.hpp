// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEBILLOWNODE_HPP
#define NOGGIT_NOISEBILLOWNODE_HPP

#include "NoiseGeneratorBase.hpp"
#include <external/libnoise/src/noise/noise.h>
#include <QComboBox>

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
        class NoiseBillowNode : public NoiseGeneratorBase
        {
        Q_OBJECT

        public:
            NoiseBillowNode();
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QComboBox* _quality;
            noise::module::Billow _module;
        };

    }

}

#endif //NOGGIT_NOISEBILLOWNODE_HPP
