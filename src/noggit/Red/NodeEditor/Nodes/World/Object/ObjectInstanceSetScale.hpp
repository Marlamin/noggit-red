// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_ONJECTINSTANCESETSCALE_HPP
#define NOGGIT_ONJECTINSTANCESETSCALE_HPP

#include <noggit/Red/NodeEditor/Nodes/ContextLogicNodeBase.hpp>

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
    class ObjectInstanceSetScaleNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      ObjectInstanceSetScaleNode();
      void compute() override;
      NodeValidationState validate() override;
    };

  }

}

#endif //NOGGIT_ONJECTINSTANCESETSCALE_HPP
