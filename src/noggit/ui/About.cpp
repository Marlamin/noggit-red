// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/About.h>

#include "revision.h"

#include <ui_About.h>

namespace noggit
{
  namespace ui
  {
    about::about(QWidget* parent)
      : QDialog(parent)
    {
      setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
      Ui::About AboutUi;
     
      AboutUi.setupUi(this);
      AboutUi.version->setText("v. " STRPRODUCTVER);
      AboutUi.build_date->setText("build date:   " __DATE__ ", " __TIME__);
    }
  }
}
