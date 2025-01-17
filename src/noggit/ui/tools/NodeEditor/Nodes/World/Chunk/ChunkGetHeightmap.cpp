// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkGetHeightmap.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>
#include <noggit/tool_enums.hpp>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ChunkGetHeightmapNode::ChunkGetHeightmapNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: GetHeightmap");
  setCaption("Chunk :: GetHeightmap");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ListData>(PortType::Out, "List[Decimal]", true);
  _out_ports[1].data_type->set_parameter_type("double");
}

void ChunkGetHeightmapNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();

  glm::vec3* heightmap = chunk->getHeightmap();

  _heightmap.clear();
  _heightmap.resize(mapbufsize);

  for (int i = 0; i < mapbufsize; ++i)
  {
    _heightmap[i] = std::make_shared<DecimalData>(heightmap[i].y);
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  std::shared_ptr<NodeData> list = std::make_shared<ListData>(&_heightmap);
  list->set_parameter_type("vec3");

  _out_ports[1].out_value = std::move(list);
  _node->onDataUpdated(1);

}

NodeValidationState ChunkGetHeightmapNode::validate()
{

  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
