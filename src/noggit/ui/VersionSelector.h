// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <QtWidgets/QDialog>
#include <ui_VersionSelector.h>

namespace Noggit
{
  namespace Ui
  {
      struct main_window;
    class versionSelector : public QDialog
    {
    public:
        versionSelector(main_window* parent = nullptr);
    };
  }
}
