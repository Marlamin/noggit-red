// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "NoiseTransformPointNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

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

  if (_last_module < 0 || _last_module != _operation->currentIndex())
  {
    switch (_operation->currentIndex())
    {
      case 0: // Translate
      {
        auto translate = new noise::module::TranslatePoint();
        _module.reset(translate);
        _last_module = 0;
        translate->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());
        glm::vec3 vec = defaultPortData<Vector3DData>(PortType::In, 1)->value();
        translate->SetTranslation(vec.x, vec.y, vec.z);
        break;
      }
      case 1: // Rotate
      {
        auto rotate = new noise::module::RotatePoint();
        _module.reset(rotate);
        _last_module = 1;
        rotate->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());
        glm::vec3 angles = defaultPortData<Vector3DData>(PortType::In, 1)->value();
        rotate->SetAngles(angles.x, angles.y, angles.z);
        break;
      }
      case 2: // Scale
      {
        auto scale_module = new noise::module::ScalePoint();
        _module.reset(scale_module);
        _last_module = 2;
        scale_module->SetSourceModule(0, *static_cast<NoiseData*>(_in_ports[0].in_value.lock().get())->value());
        glm::vec3 scale = defaultPortData<Vector3DData>(PortType::In, 1)->value();
        scale_module->SetScale(scale.x, scale.y, scale.z);
        break;
      }
    }

  }

  _out_ports[0].out_value = std::make_shared<NoiseData>(_module.get());
  _node->onDataUpdated(0);
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
