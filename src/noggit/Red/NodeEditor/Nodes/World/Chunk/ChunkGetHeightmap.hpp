// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CHUNKGETHEIGHTMAP_HPP
#define NOGGIT_CHUNKGETHEIGHTMAP_HPP

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
    class ChunkGetHeightmapNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      ChunkGetHeightmapNode();
      void compute() override;
      NodeValidationState validate() override;

    private:
        std::vector<std::shared_ptr<NodeData>> _heightmap;
    };

  }

}

#endif //NOGGIT_CHUNKGETHEIGHTMAP_HPP
