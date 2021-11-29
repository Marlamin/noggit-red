// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <functional>

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QToolBar>

#include <noggit/MapView.h>
#include <noggit/ui/font_noggit.hpp>
#include <noggit/bool_toggle_property.hpp>

namespace noggit
{
  namespace Red::ViewToolbar::Ui
  {
    class ViewToolbar: public QToolBar
    {
    public:
      ViewToolbar(MapView* mapView);

    private:
      QActionGroup _tool_group;
      void add_tool_icon(noggit::bool_toggle_property* view_state, const QString& name, const noggit::ui::font_noggit::icons& icon);
    };
  }
}
