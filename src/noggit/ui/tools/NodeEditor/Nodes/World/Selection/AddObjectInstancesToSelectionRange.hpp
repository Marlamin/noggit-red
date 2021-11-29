// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_ADDOBJECTINSTANCESTOSELECTIONRANGE_HPP
#define NOGGIT_ADDOBJECTINSTANCESTOSELECTIONRANGE_HPP

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
    class AddObjectInstancesToSelectionRangeNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      AddObjectInstancesToSelectionRangeNode();
      void compute() override;
    };

  }

}

#endif //NOGGIT_ADDOBJECTINSTANCESTOSELECTIONRANGE_HPP
