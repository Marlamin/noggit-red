// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_NOISEROTATEPOINTNODE_HPP
#define NOGGIT_NOISEROTATEPOINTNODE_HPP

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
        class NoiseTransformPointNode : public BaseNode
        {
        Q_OBJECT

        public:
            NoiseTransformPointNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            QComboBox* _operation;
            int _last_module = -1;
            std::unique_ptr<noise::module::Module> _module;
        };

    }

}

#endif //NOGGIT_NOISEROTATEPOINTNODE_HPP
