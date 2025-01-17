// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ImageMaskRandomPointsNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

#include <QRandomGenerator>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ImageMaskRandomPointsNode::ImageMaskRandomPointsNode()
: LogicNodeBase()
{
  setName("Image :: MaskRandomPoints");
  setCaption("Image :: MaskRandomPoints");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ImageData>(PortType::In, "Image", true);
  addPortDefault<IntegerData>(PortType::In, "Seed<Integer>", true);
  addPortDefault<DecimalData>(PortType::In, "Density<Decimal>", true);
  auto density = static_cast<QDoubleSpinBox*>(_in_ports[3].default_widget);
  density->setMinimum(-1.0);
  density->setValue(0.5);
  density->setMaximum(1.0001);


  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ListData>(PortType::Out, "List<Vector2D>", true);
  _out_ports[1].data_type->set_parameter_type("vec2");
}

void ImageMaskRandomPointsNode::compute()
{
  QImage* image = static_cast<ImageData*>(_in_ports[1].in_value.lock().get())->value_ptr();

  QRandomGenerator rand;
  rand.seed(defaultPortData<IntegerData>(PortType::In, 2)->value());

  _data.clear();

  double density = defaultPortData<DecimalData>(PortType::In, 3)->value();

  for (int i = 0; i < image->width(); ++i)
  {
    for (int j = 0; j < image->height(); ++j)
    {
      double random_value = rand.bounded(1.0001);
      bool chance_value = rand.bounded(1.0001) < density;
      if (random_value < qGray(image->pixelColor(i, j).rgb()) && chance_value)
      _data.push_back(std::make_shared<Vector2DData>(glm::vec2(i, j)));
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  auto list =  std::make_shared<ListData>(&_data);
  list->set_parameter_type(_out_ports[1].data_type->type().parameter_type_id);
  _out_ports[1].out_value = std::move(list);

  _node->onDataUpdated(1);
}

NodeValidationState ImageMaskRandomPointsNode::validate()
{
  if (!static_cast<ImageData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return LogicNodeBase::validate();
}

QJsonObject ImageMaskRandomPointsNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "seed");
  defaultWidgetToJson(PortType::In, 3, json_obj, "density");

  return json_obj;
}

void ImageMaskRandomPointsNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "seed");
  defaultWidgetFromJson(PortType::In, 3, json_obj, "density");
}
