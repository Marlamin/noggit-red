// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>

class MapView;

class QLineEdit;
class QListWidget;

namespace Noggit
{
  namespace Ui
  {
    class object_editor;

    class model_import : public QWidget
    {
    private:
      QListWidget* _list;
      QLineEdit* _textBox;

    public:
      model_import (Noggit::Ui::object_editor* object_editor);
      void buildModelList();
    };
  }
}
