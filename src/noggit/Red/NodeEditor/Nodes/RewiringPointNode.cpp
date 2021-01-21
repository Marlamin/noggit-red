// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "RewiringPointNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/Red/NodeEditor/Nodes/Scene/NodeScene.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace noggit::Red::NodeEditor::Nodes;
using QtNodes::Node;

RewiringPointNode::RewiringPointNode()
: BaseNode()
{
  setName("RewiringPointNode");
  _validation_state = NodeValidationState::Valid;

  addPort<AnyData>(PortType::In, "", false);
  addPort<UndefinedData>(PortType::Out, "", false);
}

void RewiringPointNode::compute()
{
  _out_ports[0].out_value = _in_ports[0].in_value.lock();
  Q_EMIT dataUpdated(0);
}

NodeValidationState RewiringPointNode::validate()
{
  if (!_in_ports[0].in_value.lock().get())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate input.");
    return _validation_state;
  }

  return _validation_state;
}

void RewiringPointNode::inputConnectionCreated(Connection const& connection)
{
  auto in_type = connection.dataType(PortType::Out);

  if (in_type.id == "any")
    return;

  _in_ports[0].data_type.reset(TypeFactory::create(in_type.id.toStdString()));
  _in_ports[0].data_type->set_parameter_type(in_type.parameter_type_id);

  _out_ports[0].data_type.reset(TypeFactory::create(in_type.id.toStdString()));
  _out_ports[0].data_type->set_parameter_type(in_type.parameter_type_id);
}

void RewiringPointNode::inputConnectionDeleted(Connection const& connection)
{
  auto scene = static_cast<NodeScene*>(_node->nodeGraphicsObject().scene());

  for (auto& con : _node->nodeState().connections(PortType::Out, 0))
  {
    scene->deleteConnection(*con.second);
  }

  _out_ports[0].data_type.reset(TypeFactory::create("undefined"));
  _in_ports[0].data_type.reset(TypeFactory::create("any"));
}

QJsonObject RewiringPointNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  auto type = _in_ports[0].data_type->type();
  json_obj["type_id"] = type.id;
  json_obj["parameter_type"] = type.parameter_type_id;

  return json_obj;
}

void RewiringPointNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  auto type_id = json_obj["type_id"].toString().toStdString();
  auto param_type_id = json_obj["parameter_type_id"].toString();

  if (type_id == "any")
    return;

  _out_ports[0].data_type.reset(TypeFactory::create(type_id));
  _out_ports[0].data_type->set_parameter_type(param_type_id);

  _in_ports[0].data_type.reset(TypeFactory::create(type_id));
  _in_ports[0].data_type->set_parameter_type(param_type_id);
}

