// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkInfoNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ChunkInfoNode::ChunkInfoNode()
: ContextNodeBase()
{
  setName("Chunk :: Info");
  setCaption("Chunk :: Info");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<ChunkData>(PortType::In, "Chunk", true);

  addPort<Vector2DData>(PortType::Out, "ChunkXY<Vector2D>", true);
  addPort<Vector2DData>(PortType::Out, "ChunkPos<Vector3D>", true);
  addPort<UnsignedIntegerData>(PortType::Out, "AreaID<UInteger>", true);
  addPort<DecimalData>(PortType::Out, "MinHeight<Decimal>", true);
  addPort<DecimalData>(PortType::Out, "MaxHeight<Decimal>", true);
  addPort<Vector3DData>(PortType::Out, "Center<Vector3D>", true);
  addPort<UnsignedIntegerData>(PortType::Out, "nTextures<UInteger>", true);
}

void ChunkInfoNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 0)->value();

  if (_out_ports[1].connected)
  {
    _out_ports[1].out_value = std::make_shared<Vector2DData>(glm::vec2(chunk->px, chunk->py));
    _node->onDataUpdated(1);
  }

  if (_out_ports[2].connected)
  {
    _out_ports[2].out_value = std::make_shared<Vector3DData>(glm::vec3(chunk->xbase, chunk->ybase, chunk->zbase));
    _node->onDataUpdated(2);
  }

  if (_out_ports[3].connected)
  {
    _out_ports[3].out_value = std::make_shared<UnsignedIntegerData>(chunk->getAreaID());
    _node->onDataUpdated(3);
  }

  if (_out_ports[4].connected)
  {
    _out_ports[4].out_value = std::make_shared<DecimalData>(chunk->getMinHeight());
    _node->onDataUpdated(4);
  }

  if (_out_ports[5].connected)
  {
    _out_ports[5].out_value = std::make_shared<DecimalData>(chunk->getMaxHeight());
    _node->onDataUpdated(5);
  }

  if (_out_ports[6].connected)
  {
    auto center = chunk->getCenter();
    _out_ports[6].out_value = std::make_shared<Vector3DData>(glm::vec3(center.x, center.y, center.z));
    _node->onDataUpdated(6);
  }

  if (_out_ports[7].connected)
  {
    auto center = chunk->getCenter();
    _out_ports[7].out_value = std::make_shared<UnsignedIntegerData>(static_cast<unsigned>(chunk->texture_set->num()));
    _node->onDataUpdated(7);
  }


}


NodeValidationState ChunkInfoNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextNodeBase::validate();
}
