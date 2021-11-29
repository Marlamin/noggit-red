// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LogicChainNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <vector>

using namespace noggit::ui::tools::NodeEditor::Nodes;

LogicChainNode::LogicChainNode()
: LogicNodeBase()
{
  setName("Logic :: Chain");
  setCaption("Logic :: Chain");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
}

void LogicChainNode::compute()
{
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic->value())
    return;

  int count = 0;
  for (auto& port : _out_ports)
  {
    port.out_value = std::make_shared<LogicData>(true);
    _node->onDataUpdated(count);
    count++;
  }
}

NodeValidationState LogicChainNode::validate()
{
  setValidationState(NodeValidationState::Valid);
  auto logic = static_cast<LogicData*>(_in_ports[0].in_value.lock().get());

  if (!logic)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: Failed to evaluate logic input");
  }

  return _validation_state;
}

void LogicChainNode::outputConnectionCreated(const Connection& connection)
{
  _out_ports[connection.getPortIndex(PortType::Out)].connected = true;

  if (_out_ports[_out_ports.size() - 1].connected)
  {
    addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
    emit portAdded(PortType::Out, _out_ports.size() - 1);
  }
}

void LogicChainNode::outputConnectionDeleted(const Connection& connection)
{
  _out_ports[connection.getPortIndex(PortType::Out)].connected = false;

  for (int i = static_cast<int>(_out_ports.size()) - 1; i != 0; --i)
  {
    if (!_out_ports[i].connected)
    {
      deletePort(PortType::Out, i);
    }
    else
    {
      addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
      emit portAdded(PortType::Out, _out_ports.size() - 1);
      break;
    }
  }

  if (_out_ports.size() == 1 && _out_ports[0].connected)
  {
    addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
    emit portAdded(PortType::Out, _out_ports.size() - 1);
  }

}


QJsonObject LogicChainNode::save() const
{
  auto json_obj = BaseNode::save();
  json_obj["n_dynamic_ports"] = static_cast<int>(_out_ports.size() - 1); // 1st port is presumed to be static

  return json_obj;
}

void LogicChainNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  for (int i = 0; i < json_obj["n_dynamic_ports"].toInt(); ++i)
  {
    addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
    emit portAdded(PortType::Out, _out_ports.size() - 1);
  }


}

void LogicChainNode::restorePostConnection(const QJsonObject& json_obj)
{
  // remove unwanted ports if node is copied without connections
  for (int i = static_cast<int>(_out_ports.size()) - 1; i != 1; --i)
  {
    if (!_out_ports[i].connected)
    {
      deletePort(PortType::Out, i);
    }
    else
    {
      addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
      emit portAdded(PortType::Out, _out_ports.size() - 1);
      break;
    }
  }

  // add empty port of required
  if (_out_ports[_out_ports.size() - 1].connected)
  {
    addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
    emit portAdded(PortType::Out, _out_ports.size() - 1);
  }

}
