// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "JSONObjectInfo.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

JSONObjectInfoNode::JSONObjectInfoNode()
: BaseNode()
{
  setName("JSON :: JSONObjectInfo");
  setCaption("JSON :: JSONObjectInfo");
  _validation_state = NodeValidationState::Valid;

  addPort<JSONData>(PortType::In, "JSONObject", true);

  addPort<BooleanData>(PortType::Out, "isEmpty<Boolean>", true);
  addPort<UnsignedIntegerData>(PortType::Out, "Size<UInteger>", true);
}

void JSONObjectInfoNode::compute()
{
  QJsonObject* json_obj = static_cast<JSONData*>(_in_ports[0].in_value.lock().get())->value_ptr();

  if (_out_ports[0].connected)
  {
    _out_ports[0].out_value = std::make_shared<BooleanData>(json_obj->isEmpty());
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    _out_ports[1].out_value = std::make_shared<UnsignedIntegerData>(json_obj->size());
    _node->onDataUpdated(1);
  }
}

NodeValidationState JSONObjectInfoNode::validate()
{
  if (!static_cast<JSONData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate json input.");
    return _validation_state;
  }

  return _validation_state;
}