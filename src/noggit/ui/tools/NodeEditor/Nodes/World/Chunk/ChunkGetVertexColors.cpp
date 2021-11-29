// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkGetVertexColors.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkGetVertexColorsNode::ChunkGetVertexColorsNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: GetVertexColors");
  setCaption("Chunk :: GetVertexColors");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ListData>(PortType::Out, "List[Color]", true);
  _out_ports[1].data_type->set_parameter_type("color");
}

void ChunkGetVertexColorsNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  glm::vec3* colors = chunk->getVertexColors();

  _colors.clear();
  _colors.resize(mapbufsize);

  for (int i = 0; i < mapbufsize; ++i)
  {
    glm::vec3& color = colors[i];
    _colors[i] = std::make_shared<ColorData>(glm::vec4(color.x, color.y, color.z, 1.0));
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  std::shared_ptr<NodeData> list = std::make_shared<ListData>(&_colors);
  list->set_parameter_type("color");

  _out_ports[1].out_value = std::move(list);
  _node->onDataUpdated(1);

}


NodeValidationState ChunkGetVertexColorsNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
