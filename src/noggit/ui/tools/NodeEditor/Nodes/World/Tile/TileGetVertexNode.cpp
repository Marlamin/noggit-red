// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetVertexNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>
#include <noggit/tool_enums.hpp>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

TileGetVertexNode::TileGetVertexNode()
: ContextLogicNodeBase()
{
  setName("Tile :: GetVertex");
  setCaption("Tile :: GetVertex");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);
  addPortDefault<Vector2DData>(PortType::In, "VertexXZ<Vector2D>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<Vector3DData>(PortType::Out, "Vertex<Vector3D>", true);
}

void TileGetVertexNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  auto xy_data = defaultPortData<Vector2DData>(PortType::In, 2);
  glm::vec2 const& xy = xy_data->value();

  glm::vec3 n_pos(0.0f, 0.0f, 0.0f);

  tile->GetVertex(xy.x, xy.y, &n_pos);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<Vector3DData>(glm::vec3(n_pos.x, n_pos.y, n_pos.z));
  _node->onDataUpdated(1);

}

NodeValidationState TileGetVertexNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
