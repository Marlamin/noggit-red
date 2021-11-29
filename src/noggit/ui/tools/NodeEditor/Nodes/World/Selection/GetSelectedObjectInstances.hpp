// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_GETSELECTEDOBJECTINSTANCES_HPP
#define NOGGIT_GETSELECTEDOBJECTINSTANCES_HPP

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
    class GetSelectedObjectInstancesNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      GetSelectedObjectInstancesNode();
      void compute() override;

    private:
        std::vector<std::shared_ptr<NodeData>> _objects;
    };

  }

}

#endif //NOGGIT_GETSELECTEDOBJECTINSTANCES_HPP
