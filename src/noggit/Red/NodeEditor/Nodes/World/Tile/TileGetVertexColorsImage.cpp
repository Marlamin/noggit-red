// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileGetVertexColorsImage.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileGetVertexColorsImageNode::TileGetVertexColorsImageNode()
: ContextLogicNodeBase()
{
  setName("Tile :: GetVertexColorsImage");
  setCaption("Tile :: GetVertexColorsImage");
  _validation_state = NodeValidationState::Valid;

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<TileData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
  addPort<ImageData>(PortType::Out, "Image", true);
}

void TileGetVertexColorsImageNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();

  QImage image(257, 257, QImage::Format_RGBA8888);

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = tile->getChunk(k, l);

      math::vector_3d* colors = chunk->getVertexColors();

      for (unsigned y = 0; y < SUM; ++y)
      {
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
      }
    }
  }

  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

  _out_ports[1].out_value = std::make_shared<ImageData>(image.scaled(1024, 1024, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  _node->onDataUpdated(1);

}


NodeValidationState TileGetVertexColorsImageNode::validate()
{

  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

