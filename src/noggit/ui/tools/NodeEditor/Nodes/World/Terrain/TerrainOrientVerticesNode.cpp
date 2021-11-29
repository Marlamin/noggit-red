// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TerrainOrientVerticesNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

TerrainOrientVerticesNode::TerrainOrientVerticesNode()
: ContextLogicNodeBase()
{
  setName("Terrain :: OrientVertices");
  setCaption("Terrain :: OrientVertices");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<DecimalData>(PortType::In, "Angle<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "Orientation<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TerrainOrientVerticesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  world->orientVertices({pos.x, pos.y, pos.z}, math::degrees(defaultPortData<DecimalData>(PortType::In, 2)->value()),
                        math::degrees(defaultPortData<DecimalData>(PortType::In, 3)->value()));

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}

