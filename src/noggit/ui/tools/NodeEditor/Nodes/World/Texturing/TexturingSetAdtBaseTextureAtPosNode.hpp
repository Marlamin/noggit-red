// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TEXTURINGSETADTBASETEXTUREATPOSNODE_HPP
#define NOGGIT_TEXTURINGSETADTBASETEXTUREATPOSNODE_HPP

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
    class TexturingSetAdtBaseTextureAtPosNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      TexturingSetAdtBaseTextureAtPosNode();
      void compute() override;
    };

  }

}

#endif //NOGGIT_TEXTURINGSETADTBASETEXTUREATPOSNODE_HPP
