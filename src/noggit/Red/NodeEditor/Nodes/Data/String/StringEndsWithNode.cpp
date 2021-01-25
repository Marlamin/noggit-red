// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "StringEndsWithNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

StringEndsWithNode::StringEndsWithNode()
: BaseNode()
{
  setName("String :: EndsWith");
  setCaption("String :: EndsWith");
  _validation_state = NodeValidationState::Valid;

  addPort<StringData>(PortType::In, "String", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPortDefault<StringData>(PortType::In, "Ending<String>", true);

  addPort<BooleanData>(PortType::Out, "Boolean", true);
}

void StringEndsWithNode::compute()
{
  _out_ports[0].out_value = std::make_shared<BooleanData>(
      QString::fromStdString(static_cast<StringData*>(_in_ports[0].in_value.lock().get())->value()).endsWith(
          defaultPortData<StringData>(PortType::In, 1)->value().c_str()));

  _node->onDataUpdated(0);
}

NodeValidationState StringEndsWithNode::validate()
{
  if (!static_cast<StringData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate string input.");
    return _validation_state;
  }

  return _validation_state;
}
