// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "CreateJSONObject.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

CreateJSONObjectNode::CreateJSONObjectNode()
: LogicNodeBase()
{
  setName("JSON :: CreateJSONObject");
  setCaption("JSON ::  :: CreateJSONObject");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<JSONData>(PortType::Out, "JSONObject", true);

}

void CreateJSONObjectNode::compute()
{
  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<JSONData>(QJsonObject());
  _node->onDataUpdated(1);
}



