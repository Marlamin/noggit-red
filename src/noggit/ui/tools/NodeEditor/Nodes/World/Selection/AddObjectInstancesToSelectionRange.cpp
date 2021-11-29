// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "AddObjectInstancesToSelectionRange.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/ContextLogicNodeBase.hpp>

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

AddObjectInstancesToSelectionRangeNode::AddObjectInstancesToSelectionRangeNode()
: ContextLogicNodeBase()
{
  setName("Selection :: AddObjectInstancesToSelectionRange");
  setCaption("Selection :: AddObjectInstancesToSelectionRange");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);
  addPortDefault<BooleanData>(PortType::In, "Deselect<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void AddObjectInstancesToSelectionRangeNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  double radius = defaultPortData<DecimalData>(PortType::In, 2)->value();
  bool deselect = defaultPortData<BooleanData>(PortType::In, 3)->value();

  world->range_add_to_selection({pos.x, pos.y, pos.z}, radius, deselect);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}



