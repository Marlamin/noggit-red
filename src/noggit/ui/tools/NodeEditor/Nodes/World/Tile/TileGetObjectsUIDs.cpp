// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetObjectsUIDs.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileGetObjectUIDsNode::TileGetObjectUIDsNode()
: ContextLogicNodeBase()
{
  setName("Tile :: GetObjectUIDsNode");
  setCaption("Tile :: GetObjectUIDs");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<TileData>(PortType::In, "Tile", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ListData>(PortType::Out, "List[UInteger]", true);
  _out_ports[0].data_type->set_parameter_type("uint");
}

void TileGetObjectUIDsNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  auto uids = tile->get_uids();

  _uids.clear();
  _uids.resize(uids->size());

  for (int i = 0; i < uids->size(); ++i)
  {
    _uids[i] = std::make_shared<UnsignedIntegerData>((*uids)[i]);
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ListData>(&_uids);
  _out_ports[1].out_value->set_parameter_type("uint");
  _node->onDataUpdated(1);

}


NodeValidationState TileGetObjectUIDsNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
