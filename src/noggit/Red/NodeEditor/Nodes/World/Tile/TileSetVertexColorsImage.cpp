// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TileSetVertexColorsImage.hpp"

#include <noggit/Red/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/Red/NodeEditor/Nodes/DataTypes/GenericData.hpp>

using namespace noggit::Red::NodeEditor::Nodes;

TileSetVertexColorsImageNode::TileSetVertexColorsImageNode()
: ContextLogicNodeBase()
{
  setName("Tile :: SetVertexColorsImage");
  setCaption("Tile :: SetVertexColorsImage");
  _validation_state = NodeValidationState::Valid;

  _operation = new QComboBox(&_embedded_widget);
  _operation->addItems({"Set", "Add", "Subtract", "Multiply"});
  addWidgetTop(_operation);

  addPort<LogicData>(PortType::In, "Logic", true);
  addPort<TileData>(PortType::In, "Logic", true);
  addPort<ImageData>(PortType::In, "Logic", true);

  addPort<LogicData>(PortType::Out, "Logic", true);
}

void TileSetVertexColorsImageNode::compute()
{
  World* world = gCurrentContext->getWorld();
  gCurrentContext->getViewport()->makeCurrent();
  opengl::context::scoped_setter const _ (::gl, gCurrentContext->getViewport()->context());

  MapTile* tile = defaultPortData<TileData>(PortType::In, 1)->value();
  QImage* image = defaultPortData<ImageData>(PortType::In, 2)->value_ptr();

  QImage* image_to_use = image;

  if (image->width() != image->height())
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: image should have square dimensions.");
    return;
  }

  QImage scaled;
  if (image->width() != 257)
  {
    scaled = image->scaled(257, 257, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image_to_use = &scaled;
  }

  unsigned const LONG{9}, SHORT{8}, SUM{LONG + SHORT}, DSUM{SUM * 2};

  for (int k = 0; k < 16; ++k)
  {
    for (int l = 0; l < 16; ++l)
    {
      MapChunk* chunk = tile->getChunk(k, l);

      math::vector_3d* colors = chunk->getVertexColors();

      for (unsigned y = 0; y < SUM; ++y)
        for (unsigned x = 0; x < SUM; ++x)
        {
          unsigned const plain {y * SUM + x};
          bool const is_virtual {static_cast<bool>(plain % 2)};

          if (is_virtual)
            continue;

          bool const erp = plain % DSUM / SUM;
          unsigned const idx {(plain - (is_virtual ? (erp ? SUM : 1) : 0)) / 2};

          switch (_operation->currentIndex())
          {
            case 0: // Set
            {
              auto color = image_to_use->pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  color.redF();
              colors[idx].y =  color.blueF();
              colors[idx].z =  color.greenF();
              break;
            }
            case 1: // Add
            {
              auto color = image_to_use->pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  std::min(1.0, std::max(0.0, colors[idx].x + color.redF()));
              colors[idx].y =  std::min(1.0, std::max(0.0, colors[idx].y + color.blueF()));
              colors[idx].z =  std::min(1.0, std::max(0.0, colors[idx].z + color.greenF()));
              break;
            }

            case 2: // Subtract
            {
              auto color = image_to_use->pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  std::min(1.0, std::max(0.0, colors[idx].x - color.redF()));
              colors[idx].y =  std::min(1.0, std::max(0.0, colors[idx].y - color.blueF()));
              colors[idx].z =  std::min(1.0, std::max(0.0, colors[idx].z - color.greenF()));
              break;
            }

            case 3: // Multiply
            {
              auto color = image_to_use->pixelColor((k * 16) + x, (l * 16) + y);
              colors[idx].x =  std::min(1.0, std::max(0.0, colors[idx].x * color.redF()));
              colors[idx].y =  std::min(1.0, std::max(0.0, colors[idx].y * color.blueF()));
              colors[idx].z =  std::min(1.0, std::max(0.0, colors[idx].z * color.greenF()));
              break;
            }
          }

        }

      chunk->updateVerticesData();
    }
  }


  _out_ports[0].out_value = std::make_shared<LogicData>(true);
  _node->onDataUpdated(0);

}


NodeValidationState TileSetVertexColorsImageNode::validate()
{
  if (!static_cast<TileData*>(_in_ports[1].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate tile input.");
    return _validation_state;
  }

  if (!static_cast<ImageData*>(_in_ports[2].in_value.lock().get()))
  {
    setValidationState(NodeValidationState::Error);
    setValidationMessage("Error: failed to evaluate image input.");
    return _validation_state;
  }

  return ContextLogicNodeBase::validate();
}

