// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CONTEXTLOGICNODEBASE_HPP
#define NOGGIT_CONTEXTLOGICNODEBASE_HPP

#include <noggit/ui/tools/NodeEditor/Nodes/LogicNodeBase.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/Context.hpp>

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
    class ContextLogicNodeBase : public LogicNodeBase
    {
    Q_OBJECT

    public:
      ContextLogicNodeBase() {};
      NodeValidationState validate() override
      {
        if (!gCurrentContext->getWorld() || !gCurrentContext->getViewport())
        {
          setValidationState(NodeValidationState::Error);
          setValidationMessage("Error: this node requires a world context to run.");
          return _validation_state;
        }

        return LogicNodeBase::validate();
      };

      QJsonObject save() const override
      {
        QJsonObject json_obj = BaseNode::save();

        for (int i = 0; i < _in_ports.size(); ++i)
        {
          defaultWidgetToJson(PortType::In, i, json_obj, _in_ports[i].caption);
        }

        return json_obj;
      };

      void restore(QJsonObject const& json_obj) override
      {
        BaseNode::restore(json_obj);

        for (int i = 0; i < _in_ports.size(); ++i)
        {
          defaultWidgetFromJson(PortType::In, i, json_obj, _in_ports[i].caption);
        }

      };

    };

  }

}

#endif //NOGGIT_CONTEXTLOGICNODEBASE_HPP
