// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SetHole.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

SetHoleNode::SetHoleNode()
: ContextLogicNodeBase()
{
  setName("Hole :: SetHole");
  setCaption("Hole :: SetHole");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Radius<Decimal>", true);
  addPortDefault<BooleanData>(PortType::In, "FullChunk<Boolean>", true);
  addPortDefault<BooleanData>(PortType::In, "Add<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SetHoleNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  double radius = defaultPortData<DecimalData>(PortType::In, 2)->value();
  bool full_chunk = defaultPortData<BooleanData>(PortType::In, 3)->value();
  bool add = defaultPortData<BooleanData>(PortType::In, 4)->value();

  world->setHole({pos.x, pos.y, pos.z}, radius, full_chunk, add);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);
}

