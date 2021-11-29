// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_DESELECTOBJECTINSTANCEBYUID_HPP
#define NOGGIT_DESELECTOBJECTINSTANCEBYUID_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>

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
    class DeselectObjectInstanceByUIDNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      DeselectObjectInstanceByUIDNode();
      void compute() override;
    };

  }

}

#endif //NOGGIT_DESELECTOBJECTINSTANCEBYUID_HPP
