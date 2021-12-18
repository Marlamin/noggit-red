// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <functional>

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QToolBar>

#include <noggit/MapView.h>
#include <noggit/ui/FontNoggit.hpp>
#include <noggit/BoolToggleProperty.hpp>

namespace Noggit
{
  namespace Ui::Tools::ViewToolbar::Ui
  {
    class ViewToolbar: public QToolBar
    {
    public:
      ViewToolbar(MapView* mapView);

    private:
      QActionGroup _tool_group;
      void add_tool_icon(Noggit::BoolToggleProperty* view_state, const QString& name, const Noggit::Ui::FontNoggit::Icons& icon);
    };
  }
}
