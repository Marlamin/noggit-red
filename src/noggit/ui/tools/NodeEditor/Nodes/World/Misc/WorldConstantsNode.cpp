// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "WorldConstantsNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/MapHeaders.h>

#include <external/NodeEditor/include/nodes/Node>

#include <cmath>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

WorldConstantsNode::WorldConstantsNode()
: ContextNodeBase()
{
  setName("Misc :: WorldConstants");
  setCaption("Misc :: WorldConstants");
  _validation_state = NodeValidationState::Valid;

  addPort<DecimalData>(PortType::Out, "TileSize<Decimal>", true);
  _out_ports[0].out_value = std::make_shared<DecimalData>(TILESIZE);
  addPort<DecimalData>(PortType::Out, "ChunkSize<Decimal>", true);
  _out_ports[1].out_value = std::make_shared<DecimalData>(CHUNKSIZE);
  addPort<DecimalData>(PortType::Out, "ChunkUnitSize<Decimal>", true);
  _out_ports[2].out_value = std::make_shared<DecimalData>(UNITSIZE);
  addPort<DecimalData>(PortType::Out, "MiniChunkSize<Decimal>", true);
  _out_ports[3].out_value = std::make_shared<DecimalData>(MINICHUNKSIZE);
  addPort<DecimalData>(PortType::Out, "TexDetailSize<Decimal>", true);
  _out_ports[4].out_value = std::make_shared<DecimalData>(TEXDETAILSIZE);
  addPort<DecimalData>(PortType::Out, "ZeroPoint<Decimal>", true);
  _out_ports[5].out_value = std::make_shared<DecimalData>(ZEROPOINT);
  addPort<DecimalData>(PortType::Out, "ChunkRadius<Decimal>", true);
  _out_ports[6].out_value = std::make_shared<DecimalData>(MAPCHUNK_RADIUS);
  addPort<DecimalData>(PortType::Out, "TileRadius<Decimal>", true);
  _out_ports[7].out_value = std::make_shared<DecimalData>(std::sqrt(std::pow(533.33333, 2) + std::pow(533.33333, 2)));
  addPort<UnsignedIntegerData>(PortType::Out, "nChunkVertices<UInteger>", true);
  _out_ports[8].out_value = std::make_shared<UnsignedIntegerData>(9 * 9 + 8 * 8);
  addPort<UnsignedIntegerData>(PortType::Out, "nTileChunks<UInteger>", true);
  _out_ports[9].out_value = std::make_shared<UnsignedIntegerData>(256);
}

void WorldConstantsNode::compute()
{
  for (int i = 0; i < _out_ports.size(); ++i)
  {
    if (_out_ports[i].connected)
      _node->onDataUpdated(i);
  }

}
