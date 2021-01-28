// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkGetHeightmapImage.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <cmath>
#include <limits>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkGetHeightmapImageNode::ChunkGetHeightmapImageNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: GetHeightmapImage");
  setCaption("Chunk :: GetHeightmapImage");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);
  addPortDefault<DecimalData>(PortType::In, "MinHeight<Decimal>", true);
  addPortDefault<DecimalData>(PortType::In, "MaxHeight<Decimal>", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ChunkGetHeightmapImageNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();
  double min_height = defaultPortData<DecimalData>(PortType::In, 2)->value();
  double max_height = defaultPortData<DecimalData>(PortType::In, 3)->value();

  if (min_height > max_height || std::fabs(std::fabs(max_height) - std::fabs(min_height)) < std::numeric_limits<double>::epsilon())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: incorrect precision bounds.");
    return;
  }

  math::vector_3d* heightmap = chunk->getHeightmap();

  QImage image(17, 17, QImage::Format_RGBA8888);

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};

  double const bias{min_height < .0 ? min_height : -min_height};
  max_height += bias;

  for (unsigned y = 0; y < SUM; ++y)
    for (unsigned x = 0; x < SUM; ++x)
    {
      unsigned const plain {y * SUM + x};
      bool const is_virtual {static_cast<bool>(plain % 2)};
      bool const erp = plain % DSUM / SUM;
      unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};
      float value = is_virtual ? (heightmap[idx].y + heightmap[idx + (erp ? SUM : 1)].y) / 2.f : heightmap[idx].y;
      value = (value + bias) / max_height;
      image.setPixelColor(x, y, QColor::fromRgbF(value, value, value, 1.0));
    }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(image));
  _node->onDataUpdated(1);

}

NodeValidationState ChunkGetHeightmapImageNode::validate()
{

  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}