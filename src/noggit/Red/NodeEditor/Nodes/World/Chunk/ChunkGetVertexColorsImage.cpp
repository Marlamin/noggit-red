// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ChunkGetVertexColorsImage.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/tool_enums.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

ChunkGetVertexColorsImageNode::ChunkGetVertexColorsImageNode()
: ContextLogicNodeBase()
{
  setName("Chunk :: GetVertexColorsImage");
  setCaption("Chunk :: GetVertexColorsImage");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<LogicData>(PortType::In, "Logic", true);
  addPortDefault<ChunkData>(PortType::In, "Chunk", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void ChunkGetVertexColorsImageNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapChunk* chunk = defaultPortData<ChunkData>(PortType::In, 1)->value();

  math::vector_3d* colors = chunk->getVertexColors();

  QImage image(17, 17, QImage::Format_RGBA8888);

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};


  for (unsigned y = 0; y < SUM; ++y)
    for (unsigned x = 0; x < SUM; ++x)
    {
      unsigned const plain {y * SUM + x};
      bool const is_virtual {static_cast<bool>(plain % 2)};
      bool const erp = plain % DSUM / SUM;
      unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};
      float r = is_virtual ? (colors[idx].x + colors[idx + (erp ? SUM : 1)].x) / 2.f : colors[idx].x;
      float g = is_virtual ? (colors[idx].y + colors[idx + (erp ? SUM : 1)].y) / 2.f : colors[idx].y;
      float b = is_virtual ? (colors[idx].z + colors[idx + (erp ? SUM : 1)].z) / 2.f : colors[idx].z;
      image.setPixelColor(x, y, QColor::fromRgbF(r, g, b, 1.0));
    }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(std::move(image));
  _node->onDataUpdated(1);

}


NodeValidationState ChunkGetVertexColorsImageNode::validate()
{
  if (!static_cast<ChunkData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate chunk input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}
