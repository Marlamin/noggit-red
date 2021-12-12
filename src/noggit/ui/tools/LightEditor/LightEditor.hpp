// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LIGHTEDITOR_HPP
#define NOGGIT_LIGHTEDITOR_HPP

#include <QWidget>
#include <noggit/MapView.h>

namespace Noggit::Ui::Tools
{
  class LightEditor : public QWidget
  {
  public:
    LightEditor(MapView* map_view, QWidget* parent = nullptr);

  private:
    MapView* _map_view;

  };
}

#endif //NOGGIT_LIGHTEDITOR_HPP
