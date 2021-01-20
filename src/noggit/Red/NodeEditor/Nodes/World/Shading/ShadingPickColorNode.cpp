// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ShadingPickColorNode.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ShadingPickColorNode::ShadingPickColorNode()
: ContextLogicNodeBase()
{
  setName("ShadingPickColorNode");
  setCaption("Shading Pick Color");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPort<Vector3DData>(PortType::In, "Pos<Vector3D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ColorData>(PortType::Out, "Color", true);
}

void ShadingPickColorNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  glm::vec3 const& pos = defaultPortData<Vector3DData>(PortType::In, 1)->value();

  auto color = world->pickShaderColor({pos.x, pos.y, pos.z});

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  Q_EMIT dataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ColorData>(glm::vec4(color.x, color.y, color.z, 1.0f));
  Q_EMIT dataUpdated(1);

}


