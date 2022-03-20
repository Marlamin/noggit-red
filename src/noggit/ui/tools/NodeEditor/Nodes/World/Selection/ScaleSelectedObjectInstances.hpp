// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_SCALESELECTEDOBJECTINSTANCES_HPP
#define NOGGIT_SCALESELECTEDOBJECTINSTANCES_HPP

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
    class ScaleSelectedObjectInstancesNode : public ContextLogicNodeBase
    {
    Q_OBJECT

    public:
      ScaleSelectedObjectInstancesNode();
      void compute() override;
      QJsonObject save() const override;
      void restore(QJsonObject const& json_obj) override;

    private:
        QComboBox* _operation;
    };

  }

}

#endif //NOGGIT_SCALESELECTEDOBJECTINSTANCES_HPP
