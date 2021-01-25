// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TERRAINCLEARHEIGHT_HPP
#define NOGGIT_TERRAINCLEARHEIGHT_HPP

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
    class TerrainClearHeightNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      TerrainClearHeightNode();
      void compute() override;
    };

  }

}

#endif //NOGGIT_TERRAINCLEARHEIGHT_HPP
