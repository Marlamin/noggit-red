// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TERRAINRAISELOWERNODE_HPP
#define NOGGIT_TERRAINRAISELOWERNODE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>
#include <QComboBox>

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
    class TerrainRaiseLowerNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      TerrainRaiseLowerNode();
      void compute() override;
      QJsonObject save() const override;
      void restore(QJsonObject const& json_obj) override;

    private:
      QComboBox* _mode;
    };

  }

}

#endif //NOGGIT_TERRAINRAISELOWERNODE_HPP
