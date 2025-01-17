// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "TexturingTilesetNode.hpp"

#include <noggit/ui/tools/NodeEditor/Nodes/BaseNode.inl>
#include <noggit/ui/tools/NodeEditor/Nodes/DataTypes/GenericData.hpp>
#include <noggit/TextureManager.h>

#include <external/NodeEditor/include/nodes/Node>

#include <QLabel>
#include <QPushButton>

using namespace Noggit::Ui::Tools::NodeEditor::Nodes;

TexturingTilesetNode::TexturingTilesetNode()
: ContextNodeBase()
{
  setName("Texturing :: Tileset");
  setCaption("Texturing :: Tileset");
  _validation_state = NodeValidationState::Valid;

  addPortDefault<StringData>(PortType::Out, "Path<String>", true);

  _image = new QLabel(&_embedded_widget);
  addWidgetBottom(_image);

  QPixmap pixmap(128, 128);
  pixmap.fill(Qt::black);
  _image->setPixmap(pixmap);
  _image->setAlignment(Qt::AlignHCenter);

  _update_btn = new QPushButton("Update", &_embedded_widget);
  addWidgetBottom(_update_btn);

  connect(_update_btn, &QPushButton::clicked,
          [=]()
          {
            std::string path = defaultPortData<StringData>(PortType::Out, 0)->value();

            if (path.empty())
              return;

            _image->setPixmap(*BLPRenderer::getInstance().render_blp_to_pixmap(path, 128, 128));
          });
}

void TexturingTilesetNode::compute()
{
  std::string path = defaultPortData<StringData>(PortType::Out, 0)->value();
  _out_ports[0].out_value = std::make_shared<StringData>(path);

  _node->onDataUpdated(0);
}

QJsonObject TexturingTilesetNode::save() const
{
  QJsonObject json_obj = BaseNode::save();

  defaultWidgetToJson(PortType::Out, 0, json_obj, "path");

  return json_obj;
}

void TexturingTilesetNode::restore(const QJsonObject& json_obj)
{
  BaseNode::restore(json_obj);

  defaultWidgetFromJson(PortType::Out, 0, json_obj, "path");
}
