// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "LoadImageNode.hpp"

#include "BaseNode.inl"
#include "Data/GenericData.hpp"

#include <QFileDialog>
#include <QImage>

using namespace noggit::Red::NodeEditor::Nodes;

LoadImageNode::LoadImageNode()
: LogicNodeBase()
{
  setName("LoadImageNode");
  setCaption("Load Image");
  _validation_state = NodeValidationState::Valid;


  addPort<StringData>(PortType::In, "Path<String>", true);
  addDefaultWidget(_in_ports[0].data_type->default_widget(&_embedded_widget), PortType::In, 0);

  auto button = new QPushButton("Load", &_embedded_widget);
  addWidgetTop(button);

  connect(button, &QPushButton::clicked
    , [=]
    {
      auto result(QFileDialog::getOpenFileName(
          nullptr, "Load image", static_cast<QLineEdit*>(_in_ports[0].default_widget)->text(), "*.png"));

      if (!result.isNull())
      {
        static_cast<QLineEdit*>(_in_ports[0].default_widget)->setText(result);
      }
    }
  );

  addPort<ImageData>(PortType::Out, "Image", true);

}

void LoadImageNode::compute()
{
  QImage image;

  if (!image.load(static_cast<QLineEdit*>(_in_ports[0].default_widget)->text(), ".png"))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to load image.");
    return;
  }

  _out_ports[0].out_value = std::make_shared<ImageData>(QPixmap::fromImage(image));

  Q_EMIT dataUpdated(0);
}

QJsonObject LoadImageNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["filepath"] = static_cast<QLineEdit*>(_in_ports[0].default_widget)->text();

  return json_obj;
}

void LoadImageNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  static_cast<QLineEdit*>(_in_ports[0].default_widget)->setText(json_obj["filepath"].toString());
}

