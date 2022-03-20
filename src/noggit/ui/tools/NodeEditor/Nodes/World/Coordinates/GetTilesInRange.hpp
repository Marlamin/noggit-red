// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETTILESINRANGE_HPP
#define NOGGIT_GETTILESINRANGE_HPP

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
    class GetTilesInRangeNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      GetTilesInRangeNode();
      void compute() override;

    private:
        std::vector<std::shared_ptr<NodeData>> _tiles;
    };

  }

}

#endif //NOGGIT_GETTILESINRANGE_HPP
