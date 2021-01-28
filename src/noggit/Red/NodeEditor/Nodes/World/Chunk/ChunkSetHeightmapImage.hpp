// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CHUNKSETHEIGHTMAPIMAGE_HPP
#define NOGGIT_CHUNKSETHEIGHTMAPIMAGE_HPP

#include <noggit/Red/NodeEditor/Nodes/ContextLogicNodeBase.hpp>
#include <QComboBox>

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
    class ChunkSetHeightmapImageNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      ChunkSetHeightmapImageNode();
      void compute() override;
      NodeValidationState validate() override;
      QJsonObject save() const override;
      void restore(QJsonObject const& json_obj) override;

    private:
        QComboBox* _operation;
    };

  }

}

#endif //NOGGIT_CHUNKSETHEIGHTMAPIMAGE_HPP
