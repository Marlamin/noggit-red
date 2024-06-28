// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetVariableLazyNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodeScene.hpp>
#include "noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp"
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;
using QtNodes::Node;

GetVariableLazyNodeBase::GetVariableLazyNodeBase()
: BaseNode()
{
  _validation_state = NodeValidationState::Valid;

  addPort<StringData>(PortType::In, "Name<String>", true);
  addDefaultWidget(_in_ports[0].data_type->default_widget(&_embedded_widget), PortType::In, 0);

  addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
}

void GetVariableLazyNodeBase::compute()
{
  auto variables = getVariableMap();

  auto variable_name = _in_ports[0].connected ? static_cast<StringData*>(_in_ports[0].in_value.lock().get())->value()
                                              : static_cast<QLineEdit*>(_in_ports[0].default_widget)->text().toStdString();

  auto it = variables->find(variable_name);

  if (it == variables->end())
  {
      auto sstream = std::stringstream();
      sstream << "Error: variable \"" << variable_name << "\" is not defined.";

    setValidationState(NodeValidationState::Error);
    setValidationMessage(sstream.str().c_str());
    return;
  }

  if (_out_ports[0].data_type->type().id != it->second.first.c_str())
  {
      auto sstream = std::stringstream();
      sstream << "Error: variable \"" << variable_name << "\" of type \"" << it->second.first << "\" does not match required type \"" << _out_ports[0].data_type->type().id.toStdString() << "\".";

    setValidationState(NodeValidationState::Error);
    setValidationMessage(sstream.str().c_str());
    return;
  }

  _out_ports[0].out_value = it->second.second;
  _node->onDataUpdated(0);

}

QJsonObject GetVariableLazyNodeBase::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["variable_name"] = static_cast<QLineEdit*>(_in_ports[0].default_widget)->text();

  return json_obj;
}

void GetVariableLazyNodeBase::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  static_cast<QLineEdit*>(_in_ports[0].default_widget)->setText(json_obj["variable_name"].toString());
}

void GetVariableLazyNodeBase::outputConnectionCreated(const Connection& connection)
{
  BaseNode::outputConnectionCreated(connection);

  auto port_index = connection.getPortIndex(PortType::Out);

  auto data_type = connection.dataType(PortType::In);

  _out_ports[0].data_type.reset(TypeFactory::create(data_type.id.toStdString()));
  _out_ports[0].caption = data_type.name;


}

void GetVariableLazyNodeBase::outputConnectionDeleted(const Connection& connection)
{
  BaseNode::outputConnectionDeleted(connection);

  auto port_index = connection.getPortIndex(PortType::Out);

  _out_ports[0].data_type.reset(TypeFactory::create("any"));
  _out_ports[0].caption = "Any";

}


// Scene scope

GetVariableLazyNode::GetVariableLazyNode()
: GetVariableLazyNodeBase()
{
  setName("Data :: GetVariableLazy");
  setCaption("Data :: GetVariableLazy");
}

VariableMap* GetVariableLazyNode::getVariableMap()
{
  return static_cast<NodeScene*>(_node->nodeGraphicsObject().scene())->getVariableMap();
}

// Context scope

GetContextVariableLazyNode::GetContextVariableLazyNode()
: GetVariableLazyNodeBase()
{
  setName("Data :: GetContextVariableLazy");
  setCaption("Data :: GetContextVariableLazy");
}

VariableMap* GetContextVariableLazyNode::getVariableMap()
{
  return gCurrentContext->getVariableMap();
}
