// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "JSONArrayInfo.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

JSONArrayInfoNode::JSONArrayInfoNode()
: BaseNode()
{
  setName("JSON :: JSONArrayInfo");
  setCaption("JSON :: JSONArrayInfo");
  _validation_state = NodeValidationState::Valid;

  addPort<JSONArrayData>(PortType::In, "JSONArray", true);

  addPort<BooleanData>(PortType::Out, "isEmpty<Boolean>", true);
  addPort<UnsignedIntegerData>(PortType::Out, "Size<UInteger>", true);
}

void JSONArrayInfoNode::compute()
{
  QJsonArray* json_array = static_cast<JSONArrayData*>(_in_ports[0].in_value.lock().get())->value_ptr();

  if (_out_ports[0].connected)
  {
    _out_ports[0].out_value = std::make_shared<BooleanData>(json_array->isEmpty());
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    _out_ports[1].out_value = std::make_shared<UnsignedIntegerData>(json_array->size());
    _node->onDataUpdated(1);
  }
}

NodeValidationState JSONArrayInfoNode::validate()
{
  if (!static_cast<JSONArrayData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json array input.");
    return _validation_state;
  }

  return _validation_state;
}

