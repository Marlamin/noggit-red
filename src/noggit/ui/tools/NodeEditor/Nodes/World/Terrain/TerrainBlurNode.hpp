// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TERRAINBLURNODE_HPP
#define NOGGIT_TERRAINBLURNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class QComboBox;

namespace Noggit
{
  namespace Ui::Tools::NodeEditor::Nodes
  {
    class TerrainBlurNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      TerrainBlurNode();
      void compute() override;
      QJsonObject save() const override;
      void restore(QJsonObject const& json_obj) override;

    private:
        QComboBox* _mode;
    };

  }

}

#endif //NOGGIT_TERRAINBLURNODE_HPP
