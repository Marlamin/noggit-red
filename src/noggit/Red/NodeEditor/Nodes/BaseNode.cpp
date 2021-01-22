#include "BaseNode.hpp"

#include <stdexcept>
#include <external/NodeEditor/include/nodes/Node>

#include <QHBoxLayout>
#include <QInputDialog>

using namespace noggit::Red::NodeEditor::Nodes;
using QtNodes::Node;

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

  auto layout = new QVBoxLayout(&_embedded_widget);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setAlignment(Qt::AlignHCenter);

  _embedded_widget_layout_top = new QVBoxLayout();
  layout->addLayout(_embedded_widget_layout_top);
  _embedded_widget_layout_top->setContentsMargins(5, 20, 5, 5);

  _embedded_widget_layout = new QVBoxLayout();
  layout->addLayout(_embedded_widget_layout);
  _embedded_widget_layout->setContentsMargins(5, 5, 5, 5);

  _embedded_widget_layout_bottom = new QVBoxLayout();
  layout->addLayout(_embedded_widget_layout_bottom);
  _embedded_widget_layout_bottom->setContentsMargins(5, 5, 5, 5);
}

std::shared_ptr<NodeData> BaseNode::outData(PortIndex port_index)
{
  return std::static_pointer_cast<NodeData>(_out_ports[port_index].out_value);
}

std::unique_ptr<NodeData>& BaseNode::dataModel(PortType port_type, PortIndex port_index)
{
  if (port_type == PortType::In)
  {
    return _in_ports[port_index].data_type;
  }
  else if (port_type == PortType::Out)
  {
    return _out_ports[port_index].data_type;
  }
  else
  {
    throw std::logic_error("Incorrect port type.");
  }
}

void BaseNode::setInData(std::shared_ptr<NodeData> data, PortIndex port_index)
{
  _in_ports[port_index].in_value = data;
}

void BaseNode::addDefaultWidget(QWidget* widget, PortType port_type, PortIndex port_index)
{
  auto row_layout = new QHBoxLayout();
  row_layout->setSpacing(0);
  _embedded_widget_layout->insertLayout(port_index, row_layout);
  auto label = new QLabel("", widget);
  label->setMaximumWidth(0);
  row_layout->addWidget(label);
  row_layout->addWidget(widget);

  if (port_type == PortType::In)
  {
    _in_ports[port_index].default_widget = widget;
  }
  if (port_type == PortType::Out)
  {
    _out_ports[port_index].default_widget = widget;
  }

  widget->adjustSize();
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

  return NodeDataType {"invalid", "Invalid"};
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

  return 0;
}

ConnectionPolicy BaseNode::portOutConnectionPolicy(PortIndex port_index) const
{
  return _out_ports[port_index].connection_policy;
}


QWidget* BaseNode::portDefaultValueWidget(PortType port_type, PortIndex port_index)
{
  if (port_type == PortType::In)
  {
    return _in_ports[port_index].default_widget;
  }
  else if (port_type == PortType::Out)
  {
    return _out_ports[port_index].default_widget;
  }

  return nullptr;
}

void BaseNode::inputConnectionDeleted(const Connection& connection)
{
  auto& port = _in_ports[connection.getPortIndex(PortType::In)];

  port.connected = false;

  auto default_widget = port.default_widget;

  if (default_widget)
    default_widget->setVisible(true);
}

void BaseNode::inputConnectionCreated(const Connection& connection)
{
  auto& port = _in_ports[connection.getPortIndex(PortType::In)];

  port.connected = true;

  auto default_widget = port.default_widget;

  if (default_widget)
    default_widget->setVisible(false);
}

void BaseNode::outputConnectionCreated(const Connection& connection)
{
  auto& port = _out_ports[connection.getPortIndex(PortType::Out)];
  port.connected = true;
}

void BaseNode::outputConnectionDeleted(const Connection& connection)
{
  auto port_index = connection.getPortIndex(PortType::Out);
  auto& port = _out_ports[port_index];

  if (connection.getNode(PortType::Out)->nodeState().connectionsRef(PortType::Out, port_index).empty())
    port.connected = false;
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
  deleteDefaultWidget(port_type, port_index);

  if (port_type == PortType::Out)
  {
    _out_ports.erase(_out_ports.begin() + port_index);
  }

  else if (port_type == PortType::In)
  {
    _in_ports.erase(_in_ports.begin() + port_index);
  }
  emit portRemoved(port_type, port_index);
}

void BaseNode::deleteDefaultWidget(PortType port_type, PortIndex port_index)
{
  if (port_type == PortType::Out)
  {
    if (!_out_ports[port_index].default_widget)
      return;

    _out_ports[port_index].default_widget = nullptr;
  }
  else if (port_type == PortType::In)
  {
    if (!_in_ports[port_index].default_widget)
      return;

    _in_ports[port_index].default_widget = nullptr;
  }

  auto item = _embedded_widget.layout()->itemAt(1)->layout()->itemAt(port_index);

  QLayoutItem *child;
  while ((child = item->layout()->takeAt(0)) != nullptr)
  {
    delete child->widget();
    delete child;
  }

  delete item->layout();
  _embedded_widget.layout()->itemAt(1)->layout()->removeItem(item);

  _embedded_widget.adjustSize();
}

void BaseNode::addWidgetTop(QWidget* widget)
{
  _embedded_widget_layout_top->addWidget(widget);
}

void BaseNode::addWidgetBottom(QWidget* widget)
{
  _embedded_widget_layout_bottom->addWidget(widget);
}

void BaseNode::defaultWidgetToJson(PortType port_type, PortIndex port_index, QJsonObject& json_obj, const QString& name) const
{
  if (port_type == PortType::In)
  {
    _in_ports[port_index].data_type->to_json(_in_ports[port_index].default_widget, json_obj, name.toStdString());
  }
  else if (port_type == PortType::Out)
  {
    _out_ports[port_index].data_type->to_json(_out_ports[port_index].default_widget, json_obj, name.toStdString());
  }
}

void BaseNode::defaultWidgetFromJson(PortType port_type, PortIndex port_index, const QJsonObject& json_obj, const QString& name)
{
  if (port_type == PortType::In)
  {
    _in_ports[port_index].data_type->from_json(_in_ports[port_index].default_widget, json_obj, name.toStdString());
  }
  else if (port_type == PortType::Out)
  {
    _out_ports[port_index].data_type->from_json(_out_ports[port_index].default_widget, json_obj, name.toStdString());
  }
}

void BaseNode::captionDoubleClicked()
{
  bool ok;
  QString text = QInputDialog::getText(&_embedded_widget, "Rename node",
                                       "Node name", QLineEdit::Normal,
                                       _caption, &ok, Qt::Dialog | Qt::FramelessWindowHint);
  if (ok && !text.isEmpty())
    setCaption(text);
}








