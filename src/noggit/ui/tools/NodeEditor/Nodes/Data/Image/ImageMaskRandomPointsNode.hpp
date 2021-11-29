// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_IMAGEMASKRANDOMPOINTSNODE_HPP
#define NOGGIT_IMAGEMASKRANDOMPOINTSNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>

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
        class ImageMaskRandomPointsNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            ImageMaskRandomPointsNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:
            std::vector<std::shared_ptr<NodeData>> _data;
        };

    }

}

#endif //NOGGIT_IMAGEMASKRANDOMPOINTSNODE_HPP
