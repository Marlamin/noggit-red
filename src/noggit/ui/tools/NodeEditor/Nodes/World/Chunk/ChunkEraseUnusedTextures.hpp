// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CHUNKERASEUNUSEDTEXTURES_HPP
#define NOGGIT_CHUNKERASEUNUSEDTEXTURES_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>

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
    class ChunkEraseUnusedTexturesNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      ChunkEraseUnusedTexturesNode();
      void compute() override;
      NodeValidationState validate() override;
    };

  }

}

#endif //NOGGIT_CHUNKERASEUNUSEDTEXTURES_HPP
