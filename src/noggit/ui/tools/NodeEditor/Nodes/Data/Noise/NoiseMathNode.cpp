// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseMathNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

NoiseMathNode::NoiseMathNode()
: BaseNode()
{
  setName("Noise :: Math");
  setCaption("Noise :: Math");
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
  noise::module::Module* first = static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value();
  noise::module::Module* second = static_cast<NoiseData*>(_in_ports[1].in_value.lock().get())->value();

  if (_last_module < 0 || _last_module != _operation->currentIndex())
  {
    switch (_operation->currentIndex())
    {
      case 0: // Add
      {
        _module.reset(new noise::module::Add());
        _last_module = 0;
        break;
      }
      case 1: // Max
      {
        _module.reset(new noise::module::Max());
        _last_module = 1;
        break;
      }
      case 2: // Min
      {
        _module.reset(new noise::module::Min());
        _last_module = 2;
        break;
      }
      case 3: // Multiply
      {
        _module.reset(new noise::module::Multiply());
        _last_module = 3;
        break;
      }
      case 4: // Power
      {
        _module.reset(new noise::module::Power());
        _last_module = 4;
        break;
      }
    }
  }

  _module->SetSourceModule(0, *first);
  _module->SetSourceModule(1, *second);

  _out_ports[0].out_value = std::make_shared<NoiseData>(_module.get());
  _node->onDataUpdated(0);
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
