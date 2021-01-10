// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicReturnNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <boost/format.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

LogicReturnNode::LogicReturnNode()
: LogicNodeBase()
{
  setName("LogicReturnNode");
  setCaption("Return Data");
  setValidationState(NodeValidationState::Valid);

  addPort<LogicData>(PortType::In, "Logic", true);
  auto label = new QLabel(&_embedded_widget);
  label->setMinimumWidth(180);
  addDefaultWidget(label, PortType::In, 0);

  addPort<AnyData>(PortType::In, "Any", true);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::In, 1);

}

void LogicReturnNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;
}

QJsonObject LogicReturnNode::save() const
{
  auto json_obj = BaseNode::save();
  json_obj["n_dynamic_ports"] = static_cast<int>(_in_ports.size() - 2); // 1st 2 ports are presumed to be static

  return json_obj;
}

void LogicReturnNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  for (int i = 0; i < json_obj["n_dynamic_ports"].toInt(); ++i)
  {
    addPort<AnyData>(PortType::In, "Any", true);
    emit portAdded(PortType::In, _in_ports.size() - 1);
  }

}

NodeValidationState LogicReturnNode::validate()
{
  setValidationState(NodeValidationState::Valid);
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input");
    return _validation_state;
  }

  for (int i = 1; i < _in_ports.size(); ++i)
  {
    if (!_in_ports[i].connected)
      continue;

    auto data_ptr = _in_ports[i].in_value.lock();

    if (!data_ptr)
    {
      setValidationState(NodeValidationState::Error);

      auto message = boost::format("Error: Failed to evaluate input of type <%s> at port %d.")
          % _in_ports[i].data_type->type().name.toStdString() % i;

      setValidationMessage(message.str().c_str());
      return _validation_state;
    }
  }

  return _validation_state;
}

void LogicReturnNode::inputConnectionCreated(const Connection& connection)
{
  PortIndex port_index = connection.getPortIndex(PortType::In);
  _in_ports[port_index].connected = true;

  if (!port_index) // no need to execute the following code for the first logic input
    return;

  auto connected_node = connection.getNode(PortType::Out);
  auto connected_model = static_cast<BaseNode*>(connected_node->nodeDataModel());
  auto data_type = connected_model->dataType(PortType::In, connection.getPortIndex(PortType::Out));

  auto connected_index = connection.getPortIndex(PortType::Out);
  auto& connected_data = connected_model->dataModel(PortType::Out, connected_index);

  _in_ports[port_index].data_type = connected_data->instantiate();
  _in_ports[port_index].caption = connected_model->portCaption(PortType::Out, connected_index);

  if (_in_ports[_in_ports.size() - 1].connected)
  {
    addPort<AnyData>(PortType::In, "Any", true);
    emit portAdded(PortType::In, _in_ports.size() - 1);
  }
}

void LogicReturnNode::inputConnectionDeleted(const Connection& connection)
{
  PortIndex port_index = connection.getPortIndex(PortType::In);
  _in_ports[port_index].connected = false;

  if (!port_index) // no need to execute the following code for the first logic input
    return;

  _in_ports[port_index].data_type = std::make_unique<AnyData>();
  _in_ports[port_index].caption = "Any";

  for (int i = static_cast<int>(_in_ports.size()) - 1; i != 1; --i)
  {
    if (!_in_ports[i].connected)
    {
      deletePort(PortType::In, i);
    }
    else
    {
      addPort<AnyData>(PortType::In, "Any", true);
      emit portAdded(PortType::In, _in_ports.size() - 1);
      break;
    }
  }

  if (_in_ports[_in_ports.size() - 1].connected)
  {
    addPort<AnyData>(PortType::In, "Any", true);
    emit portAdded(PortType::In, _in_ports.size() - 1);
  }

}


