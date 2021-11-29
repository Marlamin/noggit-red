// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkSetVertexColors.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkSetVertexColorsNode::ChunkSetVertexColorsNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: SetVertexColors");
  setCaption("Chunk :: SetVertexColors");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<ListData>(PortType::In, "List[Color]", true);
  _in_ports[2].data_type->set_parameter_type("color");

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ChunkSetVertexColorsNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  auto list = defaultPortData<ListData>(PortType::In, 2)->value();

  glm::vec3* colors = chunk->getVertexColors();

  for (int i = 0; i < mapbufsize; ++i)
  {
    glm::vec4 const& color = static_cast<ColorData*>(list->at(i).get())->value();

    colors[i].x = color.r;
    colors[i].y = color.g;
    colors[i].y = color.b;
  }

  chunk->update_vertex_colors();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState ChunkSetVertexColorsNode::validate()
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
