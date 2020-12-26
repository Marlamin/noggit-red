// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ListGetNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"
#include "Scene/NodeScene.hpp"

#include <external/NodeEditor/include/nodes/Node>

using QtNodes::Node;

using namespace noggit::Red::NodeEditor::Nodes;

ListGetNode::ListGetNode()
: LogicNodeBase()
{
  setName("ListGetNode");
  setCaption("Get");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 0);

  addPort<ListData>(PortType::In, "List[Any]", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 1);

  addPort<UnsignedIntegerData>(PortType::In, "Index<UInteger>", true);
  addDefaultWidget(_in_ports[2].data_type->default_widget(&_embedded_widget), PortType::In, 2);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<UndefinedData>(PortType::Out, "Value<Undefined>", true);

}

void ListGetNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get())->value();
  auto index_ptr = static_cast<UnsignedIntegerData*>(_in_ports[2].in_value.lock().get());;

  _out_ports[1].out_value = list->at((index_ptr ? index_ptr->value() : static_cast<QSpinBox*>(_in_ports[2].default_widget)->value()));

  Q_EMIT dataUpdated(1);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

}

NodeValidationState ListGetNode::validate()
{
  LogicNodeBase::validate();

  auto list = static_cast<ListData*>(_in_ports[1].in_value.lock().get());

  if (!list)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate list input.");

    _out_ports[0].out_value = std::make_shared<LogicData>(false);
    Q_EMIT dataUpdated(0);
  }

  return _validation_state;
}

QJsonObject ListGetNode::save() const
{
  QJsonObject json_obj = LogicNodeBase::save();

  json_obj["list_type"] = _in_ports[1].data_type->type().parameter_type_id;

  return json_obj;
}

void ListGetNode::restore(const QJsonObject& json_obj)
{
  LogicNodeBase::restore(json_obj);

  auto type_id = json_obj["list_type"].toString();
  auto type = TypeFactory::create(type_id.toStdString());

  _in_ports[1].data_type->set_parameter_type(type_id);
  _in_ports[1].caption = "List<" + type->type().name + ">";

  if (!type_id.isEmpty())
  {
    _out_ports[1].data_type.reset(type);
    _out_ports[1].caption = "Value<" + _out_ports[1].data_type->type().name + ">";
  }

}

void ListGetNode::inputConnectionCreated(const Connection& connection)
{
  BaseNode::inputConnectionCreated(connection);

  auto port_index = connection.getPortIndex(PortType::In);
  if (port_index == 1)
  {
    auto parameter_type = connection.dataType(PortType::Out).parameter_type_id;

    _in_ports[1].data_type->set_parameter_type(parameter_type);
    _in_ports[1].caption = connection.getNode(PortType::Out)->nodeDataModel()->portCaption(PortType::Out, connection.getPortIndex(PortType::Out));

    _out_ports[1].data_type.reset(TypeFactory::create(parameter_type.toStdString()));
    _out_ports[1].caption = "Value<" + _out_ports[1].data_type->type().name + ">";

    // clean up connections if list type changes
    if (_out_ports[1].connected && _out_ports[1].data_type->type().id != parameter_type)
    {
      auto this_node = connection.getNode(PortType::In);
      auto connections = this_node->nodeState().connections(PortType::Out, 1);

      for (auto& pair : connections)
      {
        if (pair.second->dataType(PortType::In).id != parameter_type)
        {
          static_cast<NodeScene*>(this_node->nodeGraphicsObject().scene())->deleteConnection(*pair.second);
        }
      }
    }
  }

}

void ListGetNode::inputConnectionDeleted(const Connection& connection)
{
  BaseNode::inputConnectionDeleted(connection);

  auto port_index = connection.getPortIndex(PortType::In);
  if  (port_index == 1)
  {
    _in_ports[1].data_type->set_parameter_type("");
    _in_ports[1].caption = "List[Any]";

    _out_ports[1].data_type.reset(TypeFactory::create("undefined"));
    _out_ports[1].caption = "Value<Undefined>";

    // clean up connections if list type changes
    if (_out_ports[1].connected)
    {
      auto this_node = connection.getNode(PortType::In);
      auto connections = this_node->nodeState().connections(PortType::Out, 1);

      for (auto& pair : connections)
      {
        static_cast<NodeScene*>(this_node->nodeGraphicsObject().scene())->deleteConnection(*pair.second);
      }
    }

  }

}

