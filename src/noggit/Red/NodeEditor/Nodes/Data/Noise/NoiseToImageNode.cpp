// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseToImageNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/libnoise/noiseutils/noiseutils.h>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseToImageNode::NoiseToImageNode()
: LogicNodeBase()
{
  setName("Noise :: ToImage");
  setCaption("Noise :: ToImage");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<Vector2DData>(PortType::In, "Resolution<Vector2D>", true);
  addPortDefault<Vector2DData>(PortType::In, "BoundsLower<Vector2D>", true);
  addPortDefault<Vector2DData>(PortType::In, "BoundsUpper<Vector2D>", true);
  addPortDefault<BooleanData>(PortType::In, "Seamless<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void NoiseToImageNode::compute()
{
  noise::module::Module* noise_module = static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())->value().get();

  utils::NoiseMap noise_map;
  utils::NoiseMapBuilderPlane map_builder;
  map_builder.SetSourceModule(*noise_module);
  map_builder.SetDestNoiseMap(noise_map);

  glm::vec2 resolution = defaultPortData<Vector2DData>(PortType::In, 2)->value();
  map_builder.SetDestSize(resolution.x, resolution.y);

  glm::vec2 bounds_lower = defaultPortData<Vector2DData>(PortType::In, 3)->value();
  glm::vec2 bounds_upper = defaultPortData<Vector2DData>(PortType::In, 4)->value();

  map_builder.SetBounds(bounds_lower.x, bounds_upper.x, bounds_lower.y, bounds_upper.y);

  map_builder.EnableSeamless(defaultPortData<BooleanData>(PortType::In, 5)->value());

  map_builder.Build();

  QImage image = QImage(QSize(resolution.x, resolution.y),  QImage::Format_RGBA64);

  for (int x = 0; x < static_cast<int>(resolution.x); ++x)
  {
    for (int y = 0; y < static_cast<int>(resolution.y); ++y)
    {
      float value = noise_map.GetValue(x, y);
      image.setPixelColor(x, y, QColor::fromRgbF(value, value, value, 1.0f));
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(image));
  _node->onDataUpdated(1);

}

NodeValidationState NoiseToImageNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error failed to evaluate node input");
  }

  return LogicNodeBase::validate();
}

QJsonObject NoiseToImageNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "resolution");
  defaultWidgetToJson(PortType::In, 3, json_obj, "bounds_lower");
  defaultWidgetToJson(PortType::In, 4, json_obj, "bounds_upper");
  defaultWidgetToJson(PortType::In, 5, json_obj, "seamless");

  return json_obj;
}

void NoiseToImageNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "resolution");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "bounds_lower");
  defaultWidgetFromJson(PortType::In, 4, json_obj, "bounds_upper");
  defaultWidgetFromJson(PortType::In, 5, json_obj, "seamless");
}
