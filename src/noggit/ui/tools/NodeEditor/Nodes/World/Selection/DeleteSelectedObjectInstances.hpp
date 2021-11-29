// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_DELETESELECTEDOBJECTINSTANCES_HPP
#define NOGGIT_DELETESELECTEDOBJECTINSTANCES_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>

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
    class DeleteSelectedObjectInstancesNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      DeleteSelectedObjectInstancesNode();
      void compute() override;
    };

  }

}

#endif //NOGGIT_DELETESELECTEDOBJECTINSTANCES_HPP
