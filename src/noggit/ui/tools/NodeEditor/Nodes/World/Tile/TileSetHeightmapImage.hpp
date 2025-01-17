// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TILESETHEIGHTMAPIMAGE_HPP
#define NOGGIT_TILESETHEIGHTMAPIMAGE_HPP

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
    class TileSetHeightmapImageNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      TileSetHeightmapImageNode();
      void compute() override;
      QJsonObject save() const override;
      void restore(QJsonObject const& json_obj) override;
      NodeValidationState validate() override;

    private:
        QComboBox* _operation;
    };

  }

}

#endif //NOGGIT_TILESETHEIGHTMAPIMAGE_HPP
