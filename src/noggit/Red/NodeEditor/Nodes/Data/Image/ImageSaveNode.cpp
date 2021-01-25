// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageSaveNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <QDir>
#include <QFileInfo>

using namespace noggit::Red::NodeEditor::Nodes;

ImageSaveNode::ImageSaveNode()
: LogicNodeBase()
{
  setName("Image :: Save");
  setCaption("Image :: Save");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<StringData>(PortType::In, "Path<String>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ImageSaveNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value();
  auto path_ptr = static_cast<StringData*>(_in_ports[2].in_value.lock().get());
  QString path = path_ptr ? path_ptr->value().c_str() : static_cast<QLineEdit*>(_in_ports[2].default_widget)->text();

  QDir path_folder = QFileInfo(path).absoluteDir();
  if(!path_folder.exists())
  {
    path_folder.mkpath(".");
  }

  if (path.isEmpty() || !image.save(path, "PNG"))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: saving image failed.");
    return;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
}

NodeValidationState ImageSaveNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject ImageSaveNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  _in_ports[2].data_type->to_json(_in_ports[2].default_widget, json_obj, "path");

  return json_obj;
}

void ImageSaveNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);
  _in_ports[2].data_type->from_json(_in_ports[2].default_widget, json_obj, "path");
}
