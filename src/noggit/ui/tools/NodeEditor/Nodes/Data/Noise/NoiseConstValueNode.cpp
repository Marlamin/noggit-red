// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseConstValueNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseConstValueNode::NoiseConstValueNode()
: NoiseGeneratorBase()
{
  setName("Noise :: ConstValue");
  setCaption("Noise :: ConstValue");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<DecimalData>(PortType::In, "Value<Decimal>", true);
  auto value = static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget);
  value->setMinimum(0.0);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseConstValueNode::compute()
{
  double value = defaultPortData<DecimalData>(PortType::In, 0)->value();
  if (!checkBounds(value, 0.0, std::numeric_limits<double>::max(), "Value"))
    return;
  _module.SetConstValue(value);

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);
  _node->onDataUpdated(0);
}
