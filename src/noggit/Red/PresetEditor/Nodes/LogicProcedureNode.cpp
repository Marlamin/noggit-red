// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicProcedureNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

#include <QComboBox>

using namespace noggit::Red::NodeEditor::Nodes;

LogicProcedureNode::LogicProcedureNode()
: LogicNodeBase()
{
  setName("LogicProcedureNode");
  setCaption("Procedure");
  setValidationState(NodeValidationState::Valid);

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

void LogicProcedureNode::compute()
{

}

QJsonObject LogicProcedureNode::save() const
{
  QJsonObject json_obj;

  return json_obj;
}

void LogicProcedureNode::restore(const QJsonObject& json_obj)
{

}

NodeValidationState LogicProcedureNode::validate()
{

}

