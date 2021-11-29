// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETTILECHUNKS_HPP
#define NOGGIT_GETTILECHUNKS_HPP

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
    class GetTileChunksNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      GetTileChunksNode();
      void compute() override;
      NodeValidationState validate() override;
    private:
        std::vector<std::shared_ptr<NodeData>> _chunks;
    };

  }

}

#endif //NOGGIT_GETTILECHUNKS_HPP
