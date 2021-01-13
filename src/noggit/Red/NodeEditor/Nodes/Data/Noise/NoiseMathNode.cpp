// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseMathNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseMathNode::NoiseMathNode()
: BaseNode()
{
  setName("NoiseMathNode");
  setCaption("Noise Math");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Add", "Max", "Min", "Multiply", "Power"});
  addWidgetTop(_operation);

  addPort<NoiseData>(PortType::In, "Noise", true);
  addPort<NoiseData>(PortType::In, "Noise", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseMathNode::compute()
{
  noise::module::Module* first = static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get();
  noise::module::Module* second = static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())->value().get();

  noise::module::Module* result = nullptr;

  switch (_operation->currentIndex())
  {
    case 0: // Add
    {
      result = new noise::module::Add();
      break;
    }
    case 1: // Max
    {
      result = new noise::module::Max();
      break;
    }
    case 2: // Min
    {
      result = new noise::module::Min();
      break;
    }
    case 3: // Multiply
    {
      result = new noise::module::Multiply();
      break;
    }
    case 4: // Power
    {
      result = new noise::module::Power();
      break;
    }
  }

  result->SetSourceModule(0, *first);
  result->SetSourceModule(1, *second);

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(result);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);
}

NodeValidationState NoiseMathNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())
  ||!static_cast<NoiseData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}

QJsonObject NoiseMathNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void NoiseMathNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  _operation->setCurrentIndex(json_obj["operation"].toInt());
}

