#ifndef NOGGIT_SRC_NOGGIT_RED_NODEEDITOR_NODES_WORLD_CHUNK_CHUNKADDDETAILDOODADS_HPP
#define NOGGIT_SRC_NOGGIT_RED_NODEEDITOR_NODES_WORLD_CHUNK_CHUNKADDDETAILDOODADS_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace noggit::ui::tools::NodeEditor::Nodes
{
  class ChunkAddDetailDoodads : public ContextLogicNodeBase
  {
    Q_OBJECT
    public:
      ChunkAddDetailDoodads();
      void compute() override;
      NodeValidationState validate() override;
  };
}

#endif//NOGGIT_SRC_NOGGIT_RED_NODEEDITOR_NODES_WORLD_CHUNK_CHUNKADDDETAILDOODADS_HPP
