// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseCurveNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseCurveNode::NoiseCurveNode()
: BaseNode()
{
  setName("Noise :: Curve");
  setCaption("Noise :: Curve");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<ListData>(PortType::In, "Points<List[Vector2D]>", true);
  _in_ports[1].data_type->set_parameter_type("vec2");

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseCurveNode::compute()
{
  _module.SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());

  auto point_list = static_cast<ListData*>(_in_ports[1].in_value.lock().get())->value();

  if (point_list->size() < 4)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: input list should contain at least four points.");
    return;
  }

  std::vector<double> points;
  for (int i = 0; i < point_list->size(); ++i)
  {
    glm::vec2 value = static_cast<Vector2DData*>(point_list->at(i).get())->value();

    if (std::find(points.begin(), points.end(), value.x) != points.end())
    {
      setValidationState(NodeValidationState::Error);
      setValidationMessage("Error: duplicate points input values found.");
      return;
    }

    _module.AddControlPoint(value.x, value.y);
  }

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);
}

NodeValidationState NoiseCurveNode::validate()
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
