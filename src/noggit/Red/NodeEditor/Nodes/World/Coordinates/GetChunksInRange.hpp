// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETCHUNKSINRANGE_HPP
#define NOGGIT_GETCHUNKSINRANGE_HPP

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
    class GetChunksInRangeNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      GetChunksInRangeNode();
      void compute() override;

    private:
        std::vector<std::shared_ptr<NodeData>> _chunks;
    };

  }

}

#endif //NOGGIT_GETCHUNKSINRANGE_HPP
