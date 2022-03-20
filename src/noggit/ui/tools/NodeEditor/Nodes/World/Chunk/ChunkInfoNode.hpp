// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CHUNKINFONODE_HPP
#define NOGGIT_CHUNKINFONODE_HPP

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
    class ChunkInfoNode : public ContextNodeBase
    {
    Q_OBJECT

    public:
      ChunkInfoNode();
      void compute() override;
      NodeValidationState validate() override;
    };

  }

}

#endif //NOGGIT_CHUNKINFONODE_HPP
