#include "LogicBeginNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

using namespace noggit::Red::PresetEditor::Nodes;

LogicBeginNode::LogicBeginNode()
: LogicNodeBase()
{
  setName("LogicBeginNode");
  setCaption("Begin");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

void LogicBeginNode::compute()
{
  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);
}
