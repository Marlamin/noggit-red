// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Toolbar.h>

namespace noggit
{
  namespace ui
  {
    toolbar::toolbar(std::function<void (editing_mode)> set_editing_mode)
      : _set_editing_mode (set_editing_mode)
      , _tool_group(this)
    {
      setContextMenuPolicy(Qt::PreventContextMenu);
      setAllowedAreas(Qt::LeftToolBarArea);

      add_tool_icon (editing_mode::ground,       tr("Raise / Lower"),     font_noggit::TOOL_RAISE_LOWER);
      add_tool_icon (editing_mode::flatten_blur, tr("Flatten / Blur"),    font_noggit::TOOL_FLATTEN_BLUR);
      add_tool_icon (editing_mode::paint,        tr("Texture Painter"),   font_noggit::TOOL_TEXTURE_PAINT);
      add_tool_icon (editing_mode::holes,        tr("Hole Cutter"),       font_noggit::TOOL_HOLE_CUTTER);
      add_tool_icon (editing_mode::areaid,       tr("Area Designator"),   font_noggit::TOOL_AREA_DESIGNATOR);
      add_tool_icon (editing_mode::flags,        tr("Impass Designator"), font_noggit::TOOL_IMPASS_DESIGNATOR);
      add_tool_icon (editing_mode::water,        tr("Water Editor"),      font_noggit::TOOL_WATER_EDITOR);
      add_tool_icon (editing_mode::mccv,         tr("Vertex Painter"),    font_noggit::TOOL_VERTEX_PAINT);
      add_tool_icon (editing_mode::object,       tr("Object Editor"),     font_noggit::TOOL_OBJECT_EDITOR);
      add_tool_icon (editing_mode::minimap,      tr("Minimap Editor"),    font_noggit::TOOL_MINIMAP_EDITOR);
      add_tool_icon(editing_mode::stamp,         tr("Stamp Mode"),        font_noggit::TOOL_STAMP);
    }

    void toolbar::add_tool_icon(editing_mode mode, const QString& name, const font_noggit::icons& icon)
    {
      auto action = addAction(font_noggit_icon{icon}, name);
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
