// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkSetHeightmap.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkSetHeightmapNode::ChunkSetHeightmapNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: SetHeightmap");
  setCaption("Chunk :: SetHeightmap");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<ListData>(PortType::In, "List[Decimal, 145]", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ChunkSetHeightmapNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  auto list = defaultPortData<ListData>(PortType::In, 2)->value();

  glm::vec3* heightmap = chunk->getHeightmap();

  for (int i = 0; i < mapbufsize; ++i)
  {
    heightmap[i].y = static_cast<DecimalData*>(list->at(i).get())->value();
  }

  chunk->updateVerticesData();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState ChunkSetHeightmapNode::validate()
{

  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }
  auto list = static_cast<ListData*>(_in_ports[2].in_value.lock().get());

  if (!list)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate list input.");
    return _validation_state;
  }

  if (list->value()->size() != 145)
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: list of heights should have 145 decimal values.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}


