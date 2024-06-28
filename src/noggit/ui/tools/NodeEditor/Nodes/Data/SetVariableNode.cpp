// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SetVariableNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodeScene.hpp>
#include "noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp"
#include <sstream>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

SetVariableNodeBase::SetVariableNodeBase()
: LogicNodeBase()
{
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPort<StringData>(PortType::In, "Name<String>", true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 1);
  addPort<AnyData>(PortType::In, "Any", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 2);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SetVariableNodeBase::compute()
{
  if (!static_cast<LogicData*>(_in_ports[0].in_value.lock().get())->value())
    return;

  auto variables = getVariableMap();

  auto variable_name = _in_ports[1].connected ? static_cast<StringData*>(_in_ports[1].in_value.lock().get())->value() : static_cast<QLineEdit*>(_in_ports[1].default_widget)->text().toStdString();

  // search for existing variable
  auto it = variables->find(variable_name);

  if (it != variables->end() && _in_ports[2].data_type->type().id != it->second.first.c_str())
  {
      auto sstream = std::stringstream();
      sstream << "Error: variable \"" << variable_name << "\" of type \"" << it->second.first << "\" does not match required type \"" << _out_ports[0].data_type->type().id.toStdString() << "\".";

    setValidationState(NodeValidationState::Error);
    setValidationMessage(sstream.str().c_str());
    return;
  }

  (*variables)[variable_name] = std::make_pair<std::string, std::shared_ptr<NodeData>>(_in_ports[2].data_type->type().id.toStdString(), _in_ports[2].in_value.lock());
  _out_ports[0].out_value = std::make_shared<LogicData>(true);

  _node->onDataUpdated(0);
}

QJsonObject SetVariableNodeBase::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["variable_name"] = static_cast<QLineEdit*>(_in_ports[1].default_widget)->text();

  return json_obj;
}

void SetVariableNodeBase::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  static_cast<QLineEdit*>(_in_ports[1].default_widget)->setText(json_obj["variable_name"].toString());
}

void SetVariableNodeBase::inputConnectionCreated(const Connection& connection)
{
  BaseNode::inputConnectionCreated(connection);

  auto port_index = connection.getPortIndex(PortType::In);

  if (port_index != 2)
    return;

  auto data_type = connection.dataType(PortType::Out);
  _in_ports[2].data_type.reset(TypeFactory::create(data_type.id.toStdString()));
  _in_ports[2].caption = data_type.name;

}

void SetVariableNodeBase::inputConnectionDeleted(const Connection& connection)
{
  BaseNode::inputConnectionDeleted(connection);

  auto port_index = connection.getPortIndex(PortType::In);

  if (port_index != 2)
    return;

  _in_ports[2].data_type.reset(TypeFactory::create("any"));
  _in_ports[2].caption = "Any";

}

NodeValidationState SetVariableNodeBase::validate()
  {
    return LogicNodeBase::validate();

  if (!_in_ports[2].connected)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: missing value input");
    return _validation_state;
  }

  return _validation_state;
}

// Scene scope

SetVariableNode::SetVariableNode()
: SetVariableNodeBase()
{
  setName("Data :: SetVariable");
  setCaption("Data :: SetVariable");
}

VariableMap* SetVariableNode::getVariableMap()
{
  return static_cast<NodeScene*>(_node->nodeGraphicsObject().scene())->getVariableMap();
}

// Context scope

SetContextVariableNode::SetContextVariableNode()
: SetVariableNodeBase()
{
  setName("Data :: SetContextVariable");
  setCaption("Data :: SetContextVariable");
}

VariableMap* SetContextVariableNode::getVariableMap()
{
  return gCurrentContext->getVariableMap();
}
