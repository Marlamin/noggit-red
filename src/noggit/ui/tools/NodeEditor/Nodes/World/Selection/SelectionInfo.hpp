// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_SELECTIONINFO_HPP
#define NOGGIT_SELECTIONINFO_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextNodeBase.hpp>

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
    class SelectionInfoNode : public ContextNodeBase
    {
    Q_OBJECT

    public:
      SelectionInfoNode();
      void compute() override;
    };

  }

}

#endif //NOGGIT_SELECTIONINFO_HPP
