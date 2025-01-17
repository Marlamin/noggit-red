// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageMirrorNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageMirrorNode::ImageMirrorNode()
: LogicNodeBase()
{
  setName("Image :: Mirror");
  setCaption("Image :: Mirror");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<BooleanData>(PortType::In, "U<Boolean>", true);
  addPortDefault<BooleanData>(PortType::In, "V<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ImageMirrorNode::compute()
{
  QImage image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value();

  QImage new_img = image.mirrored(defaultPortData<BooleanData>(PortType::In, 2)->value(),
                                    defaultPortData<BooleanData>(PortType::In, 3)->value());


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(new_img));
  _node->onDataUpdated(1);
}

NodeValidationState ImageMirrorNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

   return LogicNodeBase::validate();
}

QJsonObject ImageMirrorNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "mirror_u");
  defaultWidgetToJson(PortType::In, 3, json_obj, "mirror_v");

  return json_obj;
}

void ImageMirrorNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "mirror_u");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "mirror_v");
}
