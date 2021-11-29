// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ScaleSelectedObjectInstances.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

ScaleSelectedObjectInstancesNode::ScaleSelectedObjectInstancesNode()
: ContextLogicNodeBase()
{
  setName("Selection :: ScaleSelectedObjectInstances");
  setCaption("Selection :: ScaleSelectedObjectInstances");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Set", "Add", "Multiply"});
  addWidgetTop(_operation);

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<DecimalData>(PortType::In, "Delta<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ScaleSelectedObjectInstancesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  double delta = defaultPortData<DecimalData>(PortType::In, 1)->value();

  switch (_operation->currentIndex())
  {
    case 0: // Set
      world->scale_selected_models(delta, World::m2_scaling_type::set);
      break;

    case 1: // Add
      world->scale_selected_models(delta, World::m2_scaling_type::add);
      break;

    case 2: // Multiply
      world->scale_selected_models(delta, World::m2_scaling_type::mult);
      break;
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


QJsonObject ScaleSelectedObjectInstancesNode::save() const
{
  QJsonObject json_obj = ContextLogicNodeBase::save();

  json_obj["operation"] = _operation->currentIndex();

  return json_obj;
}

void ScaleSelectedObjectInstancesNode::restore(const QJsonObject& json_obj)
{
  ContextLogicNodeBase::restore(json_obj);

  _operation->setCurrentIndex(json_obj["operation"].toInt());
}
