// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEMATHNODE_HPP
#define NOGGIT_NOISEMATHNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.hpp>
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
        class NoiseMathNode : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseMathNode();
            NodeValidationState validate() override;
            void compute() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            std::unique_ptr<noise::module::Module> _module;
            int _last_module = -1;
            QComboBox* _operation;
        };

    }

}

#endif //NOGGIT_NOISEMATHNODE_HPP
