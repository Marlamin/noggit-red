// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListSizeNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

ListSizeNode::ListSizeNode()
: BaseNode()
{
  setName("List :: Size");
  setCaption("List :: Size");
  _validation_state = NodeValidationState::Valid;

  addPort<ListData>(PortType::In, "List[Any]", true);
  addPort<UnsignedIntegerData>(PortType::Out, "Size<UInteger>", true);
}

void ListSizeNode::compute()
{
  _out_ports[0].out_value =
      std::make_shared<UnsignedIntegerData>(static_cast<ListData*>(_in_ports[0].in_value.lock().get())->value()->size());

  _node->onDataUpdated(0);

}

NodeValidationState ListSizeNode::validate()
{
  if (!static_cast<ListData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate list input.");
    return _validation_state;
  }

  return _validation_state;
}
