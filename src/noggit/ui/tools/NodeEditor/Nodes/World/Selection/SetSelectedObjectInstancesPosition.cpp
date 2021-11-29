// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SetSelectedObjectInstancesPosition.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

SetSelectedObjectInstancesPositionNode::SetSelectedObjectInstancesPositionNode()
: ContextLogicNodeBase()
{
  setName("Selection :: SetSelectedObjectInstancesPosition");
  setCaption("Selection :: SetSelectedObjectInstancesPosition");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<Vector3DData>(PortType::In, "Pos<Vector3D>", true);
  addPortDefault<BooleanData>(PortType::In, "ChangeHeight<Boolean>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void SetSelectedObjectInstancesPositionNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto pos_data = defaultPortData<Vector3DData>(PortType::In, 1);
  glm::vec3 const& pos = pos_data->value();

  bool change_height = defaultPortData<BooleanData>(PortType::In, 2)->value();

  world->set_selected_models_pos(pos.x, pos.y, pos.z, change_height);

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


