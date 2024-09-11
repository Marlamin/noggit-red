// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <functional>

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QToolBar>

#include <noggit/tool_enums.hpp>
#include <noggit/ui/FontNoggit.hpp>

#include <unordered_map>

namespace Noggit
{
  class Tool;

  namespace Ui
  {
    class toolbar: public QToolBar
    {
    public:
      toolbar(std::vector<std::unique_ptr<Noggit::Tool>> const& tools, std::function<void (editing_mode)> set_editing_mode);

      void check_tool(editing_mode);

    private:
      std::function<void (editing_mode)> _set_editing_mode;
      QActionGroup _tool_group;

      std::unordered_map<editing_mode, QAction*> _tool_actions;

      void add_tool_icon(editing_mode mode, const QString& name, const FontNoggit::Icons& icon);
    };
  }
}
