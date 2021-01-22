// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LoadImageNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <QFileDialog>
#include <QImage>

using namespace noggit::Red::NodeEditor::Nodes;

LoadImageNode::LoadImageNode()
: LogicNodeBase()
{
  setName("Image :: Load");
  setCaption("Image :: Load");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);

  addPort<StringData>(PortType::In, "Path<String>", true);
  addDefaultWidget(_in_ports[1].data_type->default_widget(&_embedded_widget), PortType::In, 1);

  _load_button = new QPushButton("Load", &_embedded_widget);
  addWidgetTop(_load_button);

  connect(_load_button, &QPushButton::clicked
    , [=]
    {
      auto result(QFileDialog::getOpenFileName(
          nullptr, "Load image", static_cast<QLineEdit*>(_in_ports[1].default_widget)->text(), "*.png"));

      if (!result.isNull())
      {
        static_cast<QLineEdit*>(_in_ports[1].default_widget)->setText(result);
      }
    }
  );

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);

}

void LoadImageNode::compute()
{
  QImage image;
  auto path_ptr = static_cast<StringData*>(_in_ports[1].in_value.lock().get());
  QString path = path_ptr ? path_ptr->value().c_str() : static_cast<QLineEdit*>(_in_ports[1].default_widget)->text();

  if (!image.load(path, "PNG"))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to load image.");
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);

  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(image));

  Q_EMIT dataUpdated(1);
}

QJsonObject LoadImageNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["filepath"] = static_cast<QLineEdit*>(_in_ports[1].default_widget)->text();

  return json_obj;
}

void LoadImageNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  static_cast<QLineEdit*>(_in_ports[1].default_widget)->setText(json_obj["filepath"].toString());
}

void LoadImageNode::inputConnectionCreated(Connection const& connection)
{
  BaseNode::inputConnectionCreated(connection);
  PortIndex port_index = connection.getPortIndex(PortType::In);

  if (port_index != 1)
    return;

  _load_button->setHidden(true);

}

void LoadImageNode::inputConnectionDeleted(const Connection& connection)
{
  BaseNode::inputConnectionDeleted(connection);
  PortIndex port_index = connection.getPortIndex(PortType::In);

  if (port_index != 1)
    return;

  _load_button->setHidden(false);
}
