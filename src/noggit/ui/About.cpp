// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/About.h>
#include <noggit/TextureManager.h>

#include "revision.h"

#include <QIcon>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    about::about(QWidget* parent)
      : QDialog(parent)
    {
      setWindowIcon(QIcon(":/icon"));
      setWindowTitle("About");

      new QHBoxLayout (this);

      auto icon (new QLabel (this));
      auto pixmap = QPixmap::fromImage(QImage(":/icon"));
      icon->setPixmap(pixmap.scaled(128,128));

      layout()->addWidget (icon);

      layout()->addWidget (new QLabel ("Noggit Studio Red", this));
      layout()->addWidget (new QLabel ("WoW map editor for 3.3.5a", this));
      layout()->addWidget (new QLabel ("Ufoz [...], Cryect, Beket, Schlumpf, "
                                       "Tigurius, Steff, Garthog, Glararan, Cromon, "
                                       "Hanfer, Skarn, AxelSheva, Valium, Kaev, "
                                       "Adspartan", this));
      layout()->addWidget (new QLabel ("World of Warcraft is (C) Blizzard Entertainment", this));
      layout()->addWidget (new QLabel (STRPRODUCTVER, this));
      layout()->addWidget (new QLabel (__DATE__ ", " __TIME__, this));
    }
  }
}
