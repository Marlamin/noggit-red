// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "SelectionInfo.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

SelectionInfoNode::SelectionInfoNode()
: ContextNodeBase()
{
  setName("Selection :: SelectionInfo");
  setCaption("Selection :: SelectionInfo");
  _validation_state = NodeValidationState::Valid;

  addPort<BooleanData>(PortType::Out, "HasSelection<Boolean>", true);
  addPort<BooleanData>(PortType::Out, "HasMultiselection<Boolean>", true);
  addPort<UnsignedIntegerData>(PortType::Out, "nSelected<UInteger>", true);
  addPort<Vector3DData>(PortType::Out, "Pivot<Vector3D>", true);
}

void SelectionInfoNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  if (_out_ports[0].connected)
  {
    _out_ports[0].out_value = std::make_shared<BooleanData>(world->has_selection());
    _node->onDataUpdated(0);
  }

  if (_out_ports[1].connected)
  {
    _out_ports[1].out_value = std::make_shared<BooleanData>(world->has_multiple_model_selected());
    _node->onDataUpdated(1);
  }

  if (_out_ports[2].connected)
  {
    _out_ports[2].out_value = std::make_shared<UnsignedIntegerData>(world->get_selected_model_count());
    _node->onDataUpdated(2);
  }

  if (_out_ports[3].connected)
  {
    auto const& pivot = world->multi_select_pivot();

    if (!pivot)
    {
      setValidationState(NodeValidationState::Error);
      setValidationMessage("Error: multiselection pivot is not available.");
      return;
    }

    auto pivot_val = pivot.get();

    _out_ports[3].out_value = std::make_shared<Vector3DData>(glm::vec3(pivot_val.x, pivot_val.y, pivot_val.z));
    _node->onDataUpdated(3);
  }
}

