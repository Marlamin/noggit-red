// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetVariableNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodeScene.hpp>
#include "noggit/ui/tools/NodeEditor/Nodes/Scene/Context.hpp"

#include <boost/format.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace noggit::Red::NodeEditor::Nodes;
using QtNodes::Node;

GetVariableNodeBase::GetVariableNodeBase()
: LogicNodeBase()
{
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPort<StringData>(PortType::In, "Name<String>", true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 1);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
  addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
}

void GetVariableNodeBase::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  auto variables = getVariableMap();

  auto variable_name = _in_ports[1].connected ? static_cast<StringData*>(_in_ports[1].in_value.lock().get())->value()
                                              : static_cast<QLineEdit*>(_in_ports[1].default_widget)->text().toStdString();

  auto it = variables->find(variable_name);

  if (it == variables->end())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage((boost::format("Error: variable \"%s\" is not defined.") % variable_name).str().c_str());
    return;
  }

  if (_out_ports[1].data_type->type().id != it->second.first.c_str())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage((boost::format("Error: variable \"%s\" of type \"%s\" does not match required type \"%s\".")
      % variable_name % it->second.first.c_str() % _out_ports[0].data_type->type().id.toStdString()).str().c_str());
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = it->second.second;
  _node->onDataUpdated(1);

}

QJsonObject GetVariableNodeBase::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["variable_name"] = static_cast<QLineEdit*>(_in_ports[1].default_widget)->text();

  return json_obj;
}

void GetVariableNodeBase::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  static_cast<QLineEdit*>(_in_ports[1].default_widget)->setText(json_obj["variable_name"].toString());
}

void GetVariableNodeBase::outputConnectionCreated(const Connection& connection)
{
  BaseNode::outputConnectionCreated(connection);

  auto port_index = connection.getPortIndex(PortType::Out);

  if (port_index != 1)
    return;

  auto data_type = connection.dataType(PortType::In);

  _out_ports[1].data_type.reset(TypeFactory::create(data_type.id.toStdString()));
  _out_ports[1].caption = data_type.name;


}

void GetVariableNodeBase::outputConnectionDeleted(const Connection& connection)
{
  BaseNode::outputConnectionDeleted(connection);

  auto port_index = connection.getPortIndex(PortType::Out);

  if (port_index != 1)
    return;

  _out_ports[1].data_type.reset(TypeFactory::create("any"));
  _out_ports[1].caption = "Any";

}

NodeValidationState GetVariableNodeBase::validate()
{
  LogicNodeBase::validate();

  return _validation_state;
}


// Scene scope

GetVariableNode::GetVariableNode()
: GetVariableNodeBase()
{
  setName("Data :: GetVariable");
  setCaption("Data :: GetVariable");
}

VariableMap* GetVariableNode::getVariableMap()
{
  return static_cast<NodeScene*>(_node->nodeGraphicsObject().scene())->getVariableMap();
}

// Context scope

GetContextVariableNode::GetContextVariableNode()
: GetVariableNodeBase()
{
  setName("Data :: GetContextVariable");
  setCaption("Data :: GetContextVariable");
}

VariableMap* GetContextVariableNode::getVariableMap()
{
  return gCurrentContext->getVariableMap();
}
