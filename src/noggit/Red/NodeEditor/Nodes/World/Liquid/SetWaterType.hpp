// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_SETWATERTYPE_HPP
#define NOGGIT_SETWATERTYPE_HPP

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
    class SetWaterTypeNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      SetWaterTypeNode();
      void compute() override;
      QJsonObject save() const override;
      void restore(QJsonObject const& json_obj) override;

    private:
        QComboBox* _liquid_type;
    };

  }

}

#endif //NOGGIT_SETWATERTYPE_HPP
