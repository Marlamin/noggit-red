// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "GetSelectedObjectInstances.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::ui::tools::NodeEditor::Nodes;

GetSelectedObjectInstancesNode::GetSelectedObjectInstancesNode()
: ContextLogicNodeBase()
{
  setName("Selection :: GetSelectedObjectInstances");
  setCaption("Selection ::  GetSelectedObjectInstances");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ListData>(PortType::Out, "List[ObjectInstance]", true);
  _out_ports[1].data_type->set_parameter_type("object");
}

void GetSelectedObjectInstancesNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  auto const& selection = world->current_selection();

  _objects.clear();
  _objects.reserve(selection.size());

  for (int i = 0; i < selection.size(); ++i)
  {
    auto selection_entry = selection[i];

    if (selection_entry.which() != eEntry_Object)
      continue;

    _objects.push_back(std::make_shared<ObjectInstanceData>(boost::get<selected_object_type>(selection_entry)));
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ListData>(&_objects);
  _out_ports[1].out_value->set_parameter_type("object");
  _node->onDataUpdated(1);

}

