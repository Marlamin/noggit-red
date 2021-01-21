// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseTransformPointNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

NoiseTransformPointNode::NoiseTransformPointNode()
: BaseNode()
{
  setName("Noise :: TransformPoint");
  setCaption("Noise :: TransformPoint");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Translate", "Rotate", "Scale"});
  addWidgetTop(_operation);

  addPortDefault<NoiseData>(PortType::In, "Noise", true);
  addPortDefault<Vector3DData>(PortType::In, "Angles<Vector3D>", true);

  addPort<NoiseData>(PortType::Out, "Noise", true);
}

void NoiseTransformPointNode::compute()
{
  noise::module::Module* result = nullptr;

  switch (_operation->currentIndex())
  {
    case 0: // Translate
    {
      auto module = new noise::module::TranslatePoint();
      module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());
      glm::vec3 vec = defaultPortData<Vector3DData>(PortType::In, 1)->value();
      module->SetTranslation(vec.x, vec.y, vec.z);
      result = module;
      break;
    }
    case 1: // Rotate
    {
      auto module = new noise::module::RotatePoint();
      module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());
      glm::vec3 angles = defaultPortData<Vector3DData>(PortType::In, 1)->value();
      module->SetAngles(angles.x, angles.y, angles.z);
      result = module;
      break;
    }
    case 2: // Scale
    {
      auto module = new noise::module::ScalePoint();
      module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value().get());
      glm::vec3 scale = defaultPortData<Vector3DData>(PortType::In, 1)->value();
      module->SetScale(scale.x, scale.y, scale.z);
      result = module;
      break;
    }
  }

  std::shared_ptr<noise::module::Module> noise_data;
  noise_data.reset(result);
  _out_ports[0].out_value = std::make_shared<NoiseData>(noise_data);

  Q_EMIT dataUpdated(0);
}

QJsonObject NoiseTransformPointNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  json_obj["operation"] = _operation->currentIndex();
  defaultWidgetToJson(PortType::In, 1, json_obj, "vector");

  return json_obj;
}

void NoiseTransformPointNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  _operation->setCurrentIndex(json_obj["operation"].toInt());
  defaultWidgetFromJson(PortType::In, 1, json_obj, "vector");
}

NodeValidationState NoiseTransformPointNode::validate()
{
  if (!static_cast<NoiseData*>(_in_ports[0].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate noise input.");
    return _validation_state;
  }

  return _validation_state;
}
