// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoisePerlinNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <limits>

using namespace noggit::Red::NodeEditor::Nodes;

NoisePerlinNode::NoisePerlinNode()
: NoiseGeneratorBase()
{
  setName("NoisePerlinNode");
  setCaption("Perlin Noise");
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
  auto octaves = static_cast<QSpinBox*>(_in_ports[2].default_widget);
  octaves->setMinimum(0.0);
  octaves->setValue(6.0);
  octaves->setMaximum(30);

  addPortDefault<DecimalData>(PortType::In, "Persistence<Decimal>", true);
  auto persistence = static_cast<QDoubleSpinBox*>(_in_ports[3].default_widget);
  persistence->setMinimum(0.0);
  persistence->setValue(0.5);

  addPortDefault<UnsignedIntegerData>(PortType::In, "Seed<UInteger>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);

}

void NoisePerlinNode::compute()
{
  auto module = new noise::module::Perlin();

  double frequency = defaultPortData<DecimalData>(PortType::In, 0)->value();
  if (!checkBounds(frequency, 0.0, std::numeric_limits<double>::max(), "Frequency"))
    return;
  module->SetFrequency(frequency);

  double lacunarity = defaultPortData<DecimalData>(PortType::In, 1)->value();
  if (!checkBounds(lacunarity, 0.0, std::numeric_limits<double>::max(), "Lacunarity"))
    return;
  module->SetLacunarity(lacunarity);

  module->SetNoiseQuality(static_cast<noise::NoiseQuality>(_quality->currentIndex()));

  unsigned int octave_count = defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value();
  if (!checkBounds(octave_count, 0u, 30u, "Octaves"))
    return;
  module->SetOctaveCount(octave_count);

  double persistence = defaultPortData<DecimalData>(PortType::In, 3)->value();
  if (!checkBounds(persistence, 0.0, std::numeric_limits<double>::max(), "Persistence"))
    return;
  module->SetPersistence(persistence);

  module->SetSeed(defaultPortData<IntegerData>(PortType::In, 4)->value());

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(module);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);

}

QJsonObject NoisePerlinNode::save() const
{
  QJsonObject json_obj = NoiseGeneratorBase::save();

  json_obj["quality"] = _quality->currentIndex();

  return json_obj;
}

void NoisePerlinNode::restore(const QJsonObject& json_obj)
{
  NoiseGeneratorBase::restore(json_obj);

  _quality->setCurrentIndex(json_obj["quality"].toInt());
}

