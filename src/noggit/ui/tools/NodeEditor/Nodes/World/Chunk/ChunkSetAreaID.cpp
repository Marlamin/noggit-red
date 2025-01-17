// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkSetAreaID.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/ui/tools/NodeEditor/Nodes/Scene/NodesContext.hpp>
#include <noggit/tool_enums.hpp>

#include <external/NodeEditor/include/nodes/Node>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

ChunkSetAreaIDNode::ChunkSetAreaIDNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: SetAreaID");
  setCaption("Chunk :: SetAreaID");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<ChunkData>(PortType::In, "Chunk", true);
  addPort<UnsignedIntegerData>(PortType::In, "AreaID<UInteger>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void ChunkSetAreaIDNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  OpenGL::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  chunk->setAreaID(defaultPortData<UnsignedIntegerData>(PortType::In, 2)->value());


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState ChunkSetAreaIDNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
