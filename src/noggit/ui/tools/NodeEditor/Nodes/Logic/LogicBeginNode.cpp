#include "LogicBeginNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/NodeEditor/include/nodes/Node>

#include <QInputDialog>

#include <sstream>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

LogicBeginNode::LogicBeginNode()
: LogicNodeBase()
{
  setName("Logic :: Begin");
  setCaption("Logic :: Begin");
  setValidationState(NodeValidationState::Valid);
  setInterpreterToken(NodeInterpreterTokens::BEGIN);

  addPort<LogicData>(PortType::Out, "Logic", true, ConnectionPolicy::One);
  auto label = new QLabel(&_embedded_widget);
  label->setMinimumWidth(180);
  addDefaultWidget(label, PortType::Out, 0);
  addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
  addDefaultWidget(new QLabel(&_embedded_widget), PortType::Out, 1);
}

void LogicBeginNode::reset()
{
  for (int i = 0; i < _out_ports.size(); ++i)
  {
    _out_ports[i].out_value.reset();
  }
}

void LogicBeginNode::compute()
{
  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  for (int i = 1; i < _out_ports.size(); ++i)
  {
    if (!_out_ports[i].connected)
      continue;

    auto default_value = _out_ports[i].data_type->default_widget_data(_out_ports[i].default_widget);

    if (!_out_ports[i].out_value)
    {
      if (default_value)
      {
        _out_ports[i].out_value = default_value;
        _node->onDataUpdated(i);
      }
      else
      {
        setValidationState(NodeValidationState::Error);

        auto sstream = std::stringstream();
        sstream << "Error: Argument of type <" << _out_ports[i].data_type->type().name.toStdString() << "> at port " << i << " does not have a default valueand was not passed.";

        setValidationMessage(sstream.str().c_str());
        return;
      }
    }
    else
    {
      _node->onDataUpdated(i);
    }

  }
}

NodeValidationState LogicBeginNode::validate()
{
  return BaseNode::validate();
}

QJsonObject LogicBeginNode::save() const
{
  auto json_obj = BaseNode::save();
  json_obj["n_dynamic_ports"] = static_cast<int>(_out_ports.size() - 2); // 1st 2 ports are presumed to be static

  for (int i = 1; i < _out_ports.size(); ++i)
  {
    if (!_out_ports[i].connected || !_out_ports[i].default_widget)
      continue;

    auto port_name = "default_out_port_" + std::to_string(i);
    _out_ports[i].data_type->to_json(_out_ports[i].default_widget, json_obj, port_name);
    json_obj[(port_name + "_caption").c_str()] = _out_ports[i].caption;
  }

  return json_obj;
}

void LogicBeginNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  for (int i = 0; i < json_obj["n_dynamic_ports"].toInt(); ++i)
  {
    addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
    addDefaultWidget(new QLabel(&_embedded_widget), PortType::Out, 2 + i);
    emit portAdded(PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
  }

}

void LogicBeginNode::outputConnectionCreated(const Connection& connection)
{
  PortIndex port_index = connection.getPortIndex(PortType::Out);
  _out_ports[port_index].connected = true;

  if (!port_index) // no need to execute the following code for the first logic input
    return;

  auto connected_node = connection.getNode(PortType::In);
  auto connected_model = static_cast<BaseNode*>(connected_node->nodeDataModel());
  auto data_type = connected_model->dataType(PortType::In, connection.getPortIndex(PortType::In));

  auto connected_index = connection.getPortIndex(PortType::In);
  auto& connected_data = connected_model->dataModel(PortType::In, connected_index);

  _out_ports[port_index].data_type = connected_data->instantiate();
  _out_ports[port_index].data_type->set_parameter_type(data_type.parameter_type_id);

  deleteDefaultWidget(PortType::Out, port_index);
  addDefaultWidget(_out_ports[port_index].data_type->default_widget(&_embedded_widget), PortType::Out, port_index);

  _out_ports[port_index].caption = connected_model->portCaption(PortType::In, connected_index);

  if (_out_ports[_out_ports.size() - 1].connected)
  {
    addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
    addDefaultWidget(new QLabel(&_embedded_widget), PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
    emit portAdded(PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
  }

  emit visualsNeedUpdate();
}

void LogicBeginNode::outputConnectionDeleted(const Connection& connection)
{

  PortIndex port_index = connection.getPortIndex(PortType::Out);
  _out_ports[port_index].connected = false;

  if (!port_index) // no need to execute the following code for the first logic input
    return;

  _out_ports[port_index].data_type = std::make_unique<AnyData>();
  _out_ports[port_index].caption = "Any";

  deleteDefaultWidget(PortType::Out, port_index);
  addDefaultWidget(_out_ports[port_index].data_type->default_widget(&_embedded_widget), PortType::Out, port_index);

  for (int i = static_cast<int>(_out_ports.size()) - 1; i != 1; --i)
  {
    if (!_out_ports[i].connected)
    {
      deletePort(PortType::Out, i);
    }
    else
    {
      addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
      addDefaultWidget(new QLabel(&_embedded_widget), PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
      emit portAdded(PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
      break;
    }
  }

  if (_out_ports[_out_ports.size() - 1].connected)
  {
    addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
    addDefaultWidget(new QLabel(&_embedded_widget), PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
    emit portAdded(PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
  }

  emit visualsNeedUpdate();
}

void LogicBeginNode::restorePostConnection(const QJsonObject& json_obj)
{
  // restore default widget values
  for (int i = 1; i < _out_ports.size(); ++i)
  {
    if (!_out_ports[i].default_widget)
      continue;

    auto port_name = "default_out_port_" + std::to_string(i);
    _out_ports[i].data_type->from_json(_out_ports[i].default_widget, json_obj, port_name);
    _out_ports[i].caption = json_obj[(port_name + "_caption").c_str()].toString();

  }

  // remove unwanted ports if node is copied without connections
  for (int i = static_cast<int>(_out_ports.size()) - 1; i != 1; --i)
  {
    if (!_out_ports[i].connected)
    {
      deletePort(PortType::Out, i);
    }
    else
    {
      addPort<AnyData>(PortType::Out, "Any", true, ConnectionPolicy::One);
      addDefaultWidget(new QLabel(&_embedded_widget), PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
      emit portAdded(PortType::Out, static_cast<QtNodes::PortIndex>(_out_ports.size() - 1));
      break;
    }

  }
}

void LogicBeginNode::portDoubleClicked(PortType port_type, PortIndex port_index)
{
  if (port_type == PortType::In || !port_index || _out_ports[port_index].data_type->type().id == "any")
    return;

  bool ok;
  QString text = QInputDialog::getText(&_embedded_widget, "Rename port",
                                       "Port name", QLineEdit::Normal,
                                       _out_ports[port_index].caption, &ok, Qt::Dialog | Qt::FramelessWindowHint);
  if (ok && !text.isEmpty())
  {
    _out_ports[port_index].caption = text;
    Q_EMIT visualsNeedUpdate();
  }

}
