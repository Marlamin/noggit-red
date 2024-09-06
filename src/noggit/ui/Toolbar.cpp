// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Tool.hpp>
#include <noggit/ui/Toolbar.h>

namespace Noggit
{
  namespace Ui
  {
    toolbar::toolbar(std::vector<std::unique_ptr<Noggit::Tool>> const& tools, std::function<void (editing_mode)> set_editing_mode)
      : _set_editing_mode (set_editing_mode)
      , _tool_group(this)
    {
      setContextMenuPolicy(Qt::PreventContextMenu);
      setAllowedAreas(Qt::LeftToolBarArea);
      setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

      for (auto&& tool : tools)
      {
          add_tool_icon(tool->editingMode(), tr(tool->name()), tool->icon());
      }
    }

    void toolbar::add_tool_icon(editing_mode mode, const QString& name, const FontNoggit::Icons& icon)
    {
      auto action = addAction(FontNoggitIcon{icon}, name);
      connect (action, &QAction::triggered, [this, mode] () {
        _set_editing_mode (mode);
      });
      action->setActionGroup(&_tool_group);
      action->setCheckable(true);
    }

    void toolbar::check_tool(editing_mode mode)
    {
      _tool_group.actions()[static_cast<std::size_t> (mode)]->setChecked(true);
    }
  }
}
