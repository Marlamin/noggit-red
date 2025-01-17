// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CONTEXTLOGICNODEBASE_HPP
#define NOGGIT_CONTEXTLOGICNODEBASE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>

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
    class ContextLogicNodeBase : public LogicNodeBase
    {
    Q_OBJECT

    public:
      ContextLogicNodeBase();

      NodeValidationState validate() override;

      QJsonObject save() const override;

      void restore(QJsonObject const& json_obj) override;
    };
  }
}

#endif //NOGGIT_CONTEXTLOGICNODEBASE_HPP
