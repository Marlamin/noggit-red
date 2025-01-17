// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseBillowNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

#include <external/NodeEditor/include/nodes/Node>

#include <QComboBox>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

NoiseBillowNode::NoiseBillowNode()
: NoiseGeneratorBase()
{
  setName("Noise :: Billow");
  setCaption("Noise :: Billow");
  _validation_state = NodeValidationState::Valid;

  _quality = new QComboBox(&_embedded_widget);
  _quality->addItems({"Fast", "Standard", "Best"});
  _quality->setCurrentIndex(1);
  addWidgetTop(_quality);

  addPortDefault<DecimalData>(PortType::In, "Frequency<Decimal>", true);
  auto frequency = static_cast<QDoubleSpinBox*>(_in_ports[0].default_widget);
  frequency->setMinimum(0.0);
  frequency->setValue(1.0);

  addPortDefault<DecimalData>(PortType::In, "Lacunarity<Decimal>", true);
  auto lacunarity = static_cast<QDoubleSpinBox*>(_in_ports[1].default_widget);
  lacunarity->setMinimum(0.0);
  lacunarity->setValue(2.0);

  addPortDefault<UnsignedIntegerData>(PortType::In, "Octaves<UInteger>", true);
  auto octaves = static_cast<QUnsignedSpinBox*>(_in_ports[2].default_widget);
  octaves->setMinimum(1);
  octaves->setValue(6);
  octaves->setMaximum(30);

  addPortDefault<DecimalData>(PortType::In, "Persistence<Decimal>", true);
  auto persistence = static_cast<QDoubleSpinBox*>(_in_ports[3].default_widget);
  persistence->setMinimum(0.0);
  persistence->setValue(0.5);

  addPortDefault<UnsignedIntegerData>(PortType::In, "Seed<Integer>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseBillowNode::compute()
{

  double frequency = defaultPortData<DecimalData>(PortType::In, 0)->value();
  if (!checkBounds(frequency, 0.0, std::numeric_limits<double>::max(), "Frequency"))
    return;
  _module.SetFrequency(frequency);

  double lacunarity = defaultPortData<DecimalData>(PortType::In, 1)->value();
  if (!checkBounds(lacunarity, 0.0, std::numeric_limits<double>::max(), "Lacunarity"))
    return;
  _module.SetLacunarity(lacunarity);

  _module.SetNoiseQuality(static_cast<noise::NoiseQuality>(_quality->currentIndex()));

  unsigned int octave_count = defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value();
  if (!checkBounds(octave_count, 1u, 30u, "Octaves"))
    return;
  _module.SetOctaveCount(octave_count);

  double persistence = defaultPortData<DecimalData>(PortType::In, 3)->value();
  if (!checkBounds(persistence, 0.0, std::numeric_limits<double>::max(), "Persistence"))
    return;
  _module.SetPersistence(persistence);

  _module.SetSeed(defaultPortData<IntegerData>(PortType::In, 4)->value());

  _out_ports[0].out_value = std::make_shared<NoiseData>(&_module);

  _node->onDataUpdated(0);
}

QJsonObject NoiseBillowNode::save() const
{
  QJsonObject json_obj = NoiseGeneratorBase::save();

  json_obj["quality"] = _quality->currentIndex();

  return json_obj;
}

void NoiseBillowNode::restore(const QJsonObject& json_obj)
{
  NoiseGeneratorBase::restore(json_obj);

  _quality->setCurrentIndex(json_obj["quality"].toInt());
}
