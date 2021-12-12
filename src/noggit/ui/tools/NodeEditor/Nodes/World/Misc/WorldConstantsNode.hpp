// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_WORLDCONSTANTSNODE_HPP
#define NOGGIT_WORLDCONSTANTSNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextNodeBase.hpp>

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
    class WorldConstantsNode : public ContextNodeBase
    {
    Q_OBJECT

    public:
      WorldConstantsNode();
      void compute() override;
    };

  }

}

#endif //NOGGIT_WORLDCONSTANTSNODE_HPP
