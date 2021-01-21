// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "StringSizeNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

StringSizeNode::StringSizeNode()
: BaseNode()
{
  setName("String :: Size");
  setCaption("String :: Size");
  _validation_state = NodeValidationState::Valid;

  addPort<StringData>(PortType::In, "String", true);
  addPort<UnsignedIntegerData>(PortType::Out, "Size<UInteger>", true);
}

void StringSizeNode::compute()
{
  _out_ports[0].out_value = std::make_shared<UnsignedIntegerData>(static_cast<StringData*>(_in_ports[0].in_value.lock().get())->value().size());

  Q_EMIT dataUpdated(0);
}

NodeValidationState StringSizeNode::validate()
{
  if (!static_cast<StringData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate string input.");
    return _validation_state;
  }

  return _validation_state;
}
