// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TILEGETOBJECTSUIDS_HPP
#define NOGGIT_TILEGETOBJECTSUIDS_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>
#include <vector>

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
    class TileGetObjectUIDsNode : public  ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      TileGetObjectUIDsNode();
      void compute() override;
      NodeValidationState validate() override;

    private:
        std::vector<std::shared_ptr<NodeData>> _uids;
    };

  }

}

#endif //NOGGIT_TILEGETOBJECTSUIDS_HPP
