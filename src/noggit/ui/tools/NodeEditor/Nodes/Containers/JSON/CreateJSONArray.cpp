// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "CreateJSONArray.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

CreateJSONArrayNode::CreateJSONArrayNode()
: LogicNodeBase()
{
  setName("JSON :: CreateJSONArray");
  setCaption("JSON :: CreateJSONArray");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<JSONArrayData>(PortType::Out, "JSONArray", true);
}

void CreateJSONArrayNode::compute()
{
  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<JSONArrayData>(QJsonArray());
  _node->onDataUpdated(1);
}

