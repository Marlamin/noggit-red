// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TerrainClearVertexSelection.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

TerrainClearVertexSelectionNode::TerrainClearVertexSelectionNode()
: ContextLogicNodeBase()
{
  setName("Terrain :: ClearVertexSelection");
  setCaption("Terrain :: ClearVertexSelection");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TerrainClearVertexSelectionNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  world->clearVertexSelection();

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}




