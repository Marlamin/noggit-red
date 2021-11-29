// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "DeleteVariableNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodeScene.hpp>
#include "noggit/ui/tools/NodeEditor/Nodes/Scene/Context.hpp"

#include <boost/format.hpp>
#include <external/NodeEditor/include/nodes/Node>


using namespace noggit::ui::tools::NodeEditor::Nodes;
using QtNodes::Node;

DeleteVariableNodeBase::DeleteVariableNodeBase()
: LogicNodeBase()
{
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);
  addPort<StringData>(PortType::In, "Name<String>", true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 1);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void DeleteVariableNodeBase::compute()
{
  if (!static_cast<LogicData*>(_in_ports[0].in_value.lock().get())->value())
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

  variables->erase(it);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
}

QJsonObject DeleteVariableNodeBase::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["variable_name"] = static_cast<QLineEdit*>(_in_ports[1].default_widget)->text();

  return json_obj;
}

void DeleteVariableNodeBase::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  static_cast<QLineEdit*>(_in_ports[1].default_widget)->setText(json_obj["variable_name"].toString());
}


// Scene scope

DeleteVariableNode::DeleteVariableNode()
: DeleteVariableNodeBase()
{
  setName("Data :: DeleteVariable");
  setCaption("Data :: DeleteVariable");
}

VariableMap* DeleteVariableNode::getVariableMap()
{
  return static_cast<NodeScene*>(_node->nodeGraphicsObject().scene())->getVariableMap();
}

// Context scope

DeleteContextVariableNode::DeleteContextVariableNode()
: DeleteVariableNodeBase()
{
  setName("Data :: DeleteContextVariable");
  setCaption("Data :: DeleteContextVariable");
}

VariableMap* DeleteContextVariableNode::getVariableMap()
{
  return gCurrentContext->getVariableMap();
}
