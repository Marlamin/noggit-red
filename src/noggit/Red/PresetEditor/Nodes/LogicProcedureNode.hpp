// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LOGICPROCEDURENODE_HPP
#define NOGGIT_LOGICPROCEDURENODE_HPP

#include "LogicNodeBase.hpp"
#include "Scene/NodeScene.hpp"

#include <vector>

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
        class LogicProcedureNode : public LogicNodeBase
        {
        Q_OBJECT

        public:
            LogicProcedureNode();
            void compute() override;
            NodeValidationState validate() override;
            QJsonObject save() const override;
            void restore(QJsonObject const& json_obj) override;

        private:

          void clearDynamicPorts();

          QWidget* _procedure_default;
          NodeScene* _scene;
          QString _scene_path;
        };

    }

}

#endif //NOGGIT_LOGICPROCEDURENODE_HPP
