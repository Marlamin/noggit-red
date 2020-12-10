#include "BaseNode.hpp"

#include <stdexcept>

#include <QHBoxLayout>

using namespace noggit::Red::PresetEditor::Nodes;

InNodePort::InNodePort(QString const& caption_, bool caption_visible_)
: caption(caption_)
, caption_visible(caption_visible_)
{}

OutNodePort::OutNodePort(QString const& caption_, bool caption_visible_)
: caption(caption_)
, caption_visible(caption_visible_)
{}

BaseNode::BaseNode()
: NodeDataModel()
, _embedded_widget(QWidget())
{
  _embedded_widget.setAttribute(Qt::WA_TranslucentBackground);
  _embedded_widget_layout = new QVBoxLayout(&_embedded_widget);
  _embedded_widget_layout->setContentsMargins(5, 5, 5, 5);
}

std::shared_ptr<NodeData> BaseNode::outData(PortIndex port_index)
{
  return std::static_pointer_cast<NodeData>(_out_ports[port_index].out_value);
}

void BaseNode::setInData(std::shared_ptr<NodeData> data, PortIndex port_index)
{
  _in_ports[port_index].in_value = data;
}

void BaseNode::addWidget(QWidget* widget, PortIndex in_port)
{
  auto row_layout = new QHBoxLayout(&_embedded_widget);
  row_layout->setSpacing(0);
  _embedded_widget_layout->addLayout(row_layout);
  auto spacer = new QLabel("", &_embedded_widget);
  spacer->setMaximumWidth(0);
  row_layout->addWidget(spacer);
  row_layout->addWidget(widget);

  if (in_port >= 0)
  {
    _in_ports[in_port].default_widget = widget;
  }
}

void BaseNode::addWidget(QWidget* widget, QString const& label_text, PortIndex in_port)
{
  auto row_layout = new QHBoxLayout(&_embedded_widget);
  _embedded_widget_layout->addLayout(row_layout);
  row_layout->addWidget(new QLabel(label_text, &_embedded_widget));
  row_layout->addWidget(widget);

  if (in_port >= 0)
  {
    _in_ports[in_port].default_widget = widget;
  }
}

bool BaseNode::portCaptionVisible(PortType port_type, PortIndex port_index) const
{
  if (port_type == PortType::In)
  {
    return _in_ports[port_index].caption_visible;
  }
  else if (port_type == PortType::Out)
  {
    return _out_ports[port_index].caption_visible;
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }
}

QString BaseNode::portCaption(PortType port_type, PortIndex port_index) const
{
  if (port_type == PortType::In)
  {
    return _in_ports[port_index].caption;
  }
  else if (port_type == PortType::Out)
  {
    return _out_ports[port_index].caption;
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }
}

NodeDataType BaseNode::dataType(PortType port_type, PortIndex port_index) const
{
  if (port_type == PortType::In)
  {
    return port_index < _in_ports.size() ? _in_ports[port_index].data_type->type() : NodeDataType {"invalid", "Invalid"};
  }
  else if (port_type == PortType::Out)
  {
    return port_index < _out_ports.size() ? _out_ports[port_index].data_type->type() : NodeDataType {"invalid", "Invalid"};
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }
}

unsigned int BaseNode::nPorts(PortType port_type) const
{
  if (port_type == PortType::In)
  {
    return _in_ports.size();
  }
  else if (port_type == PortType::Out)
  {
    return _out_ports.size();
  }
  else
  {
    throw std::logic_error("Incorrect port type or port type None.");
  }
}

ConnectionPolicy BaseNode::portOutConnectionPolicy(PortIndex port_index) const
{
  return _out_ports[port_index].connection_policy;
}


QWidget* BaseNode::portDefaultValueWidget(PortIndex port_index)
{
  return _in_ports[port_index].default_widget;
}

void BaseNode::inputConnectionDeleted(const Connection& connection)
{
  auto default_widget = _in_ports[connection.getPortIndex(PortType::In)].default_widget;

  if (default_widget)
    default_widget->setVisible(true);
}

void BaseNode::inputConnectionCreated(const Connection& connection)
{
  auto default_widget = _in_ports[connection.getPortIndex(PortType::In)].default_widget;

  if (default_widget)
    default_widget->setVisible(false);
}

QJsonObject BaseNode::save() const
{
  QJsonObject json_obj;

  json_obj["name"] = name();
  json_obj["caption"] = caption();

  return json_obj;
}

void BaseNode::restore(const QJsonObject& json_obj)
{
  setName(json_obj["name"].toString());
  setCaption(json_obj["caption"].toString());
}

void BaseNode::deletePort(PortType port_type, PortIndex port_index)
{
  if (port_type == PortType::Out)
    _out_ports.erase(_out_ports.begin() + port_index);
  else if (port_type == PortType::In)
    _in_ports.erase(_in_ports.begin() + port_index);

  emit portRemoved();
}




