// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkSetHeightmapImage.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkSetHeightmapImageNode::ChunkSetHeightmapImageNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: SetHeightmapImage");
  setCaption("Chunk :: SetHeightmapImage");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Set", "Add", "Subtract", "Multiply"});
  addWidgetTop(_operation);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<DecimalData>(PortType::In, "Multiplier", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ChunkSetHeightmapImageNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  QImage* image = defaultPortData<ImageData>(PortType::In, 2)->value_ptr();
  double multiplier = defaultPortData<DecimalData>(PortType::In, 3)->value();

  QImage* image_to_use = image;

  if (image->width() != image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: image should have square dimensions.");
    return;
  }

  QImage scaled;
  if (image->width() != 17)
  {
    scaled = image->scaled(17, 17, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image_to_use = &scaled;
  }

  chunk->setHeightmapImage(*image_to_use, static_cast<float>(multiplier), _operation->currentIndex());

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

NodeValidationState ChunkSetHeightmapImageNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  if (!static_cast<ImageData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

QJsonObject ChunkSetHeightmapImageNode::save() const
{
  QJsonObject json_obj = ContextLogicNodeBase::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void ChunkSetHeightmapImageNode::restore(QJsonObject const& json_obj)
{
  _operation->setCurrentIndex(json_obj["operation"].toInt());

  ContextLogicNodeBase::restore(json_obj);
}