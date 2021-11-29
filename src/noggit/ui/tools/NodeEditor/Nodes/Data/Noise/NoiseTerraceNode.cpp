// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseTerraceNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

NoiseTerraceNode::NoiseTerraceNode()
: BaseNode()
{
  setName("Noise :: Terrace");
  setCaption("Noise :: Terrace");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<ListData>(PortType::In, "Terraces<List[Decimal]>", true);
  _in_ports[1].data_type->set_parameter_type("decimal");
  addPortDefault<BooleanData>(PortType::In, "Invert<Boolean>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseTerraceNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());

  auto point_list = static_cast<ListData*>(_in_ports[1].in_value.lock().get())->value();

  if (point_list->size() < 2)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: input list should contain at least two terrace heights.");
    return;
  }

  std::vector<double> points;
  for (int i = 0; i < point_list->size(); ++i)
  {
    double value = static_cast<DecimalData*>(point_list->at(i).get())->value();

    if (std::find(points.begin(), points.end(), value) != points.end())
    {
      setValidationState(NodeValidationState::Error);
      setValidationMessage("Error: duplicate terrace heights found.");
      return;
    }

    _module.AddControlPoint(value);
  }
  _module.InvertTerraces(defaultPortData<BooleanData>(PortType::In, 2)->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);

}

NodeValidationState NoiseTerraceNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  if (!static_cast<ListData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate list input.");
    return _validation_state;
  }

  return _validation_state;
}


QJsonObject NoiseTerraceNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::In, 2, json_obj, "invert");


  return json_obj;
}

void NoiseTerraceNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::In, 2, json_obj, "invert");
}
