// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TEXTURINGTILESETNODE_HPP
#define NOGGIT_TEXTURINGTILESETNODE_HPP

#include <noggit/Red/NodeEditor/Nodes/ContextNodeBase.hpp>
#include <QLabel>
#include <QPushButton>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


namespace noggit
{
  namespace Red::NodeEditor::Nodes
  {
    class TexturingTilesetNode : public ContextNodeBase
    {
    Q_OBJECT

    public:
      TexturingTilesetNode();
      void compute() override;
      QJsonObject save() const override;
      void restore(QJsonObject const& json_obj) override;

    private:
      QLabel* _image;
      QPushButton* _update_btn;
    };

  }

}

#endif //NOGGIT_TEXTURINGTILESETNODE_HPP
