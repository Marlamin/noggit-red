// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Toolbar.h>

namespace Noggit
{
  namespace Ui
  {
    toolbar::toolbar(std::function<void (editing_mode)> set_editing_mode)
      : _set_editing_mode (set_editing_mode)
      , _tool_group(this)
    {
      setContextMenuPolicy(Qt::PreventContextMenu);
      setAllowedAreas(Qt::LeftToolBarArea);
      setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

      add_tool_icon(editing_mode::ground, tr("Raise / Lower"), FontNoggit::TOOL_RAISE_LOWER);
      add_tool_icon(editing_mode::flatten_blur, tr("Flatten / Blur"), FontNoggit::TOOL_FLATTEN_BLUR);
      add_tool_icon(editing_mode::paint, tr("Texture Painter"), FontNoggit::TOOL_TEXTURE_PAINT);
      add_tool_icon(editing_mode::holes, tr("Hole Cutter"), FontNoggit::TOOL_HOLE_CUTTER);
      add_tool_icon(editing_mode::areaid, tr("Area Designator"), FontNoggit::TOOL_AREA_DESIGNATOR);
      add_tool_icon(editing_mode::flags, tr("Impass Designator"), FontNoggit::TOOL_IMPASS_DESIGNATOR);
      add_tool_icon(editing_mode::water, tr("Water Editor"), FontNoggit::TOOL_WATER_EDITOR);
      add_tool_icon(editing_mode::mccv, tr("Vertex Painter"), FontNoggit::TOOL_VERTEX_PAINT);
      add_tool_icon(editing_mode::object, tr("Object Editor"), FontNoggit::TOOL_OBJECT_EDITOR);
      add_tool_icon(editing_mode::minimap, tr("Minimap Editor"), FontNoggit::TOOL_MINIMAP_EDITOR);
      add_tool_icon(editing_mode::stamp, tr("Stamp Mode"), FontNoggit::TOOL_STAMP);
      add_tool_icon(editing_mode::light, tr("Light Editor"), FontNoggit::TOOL_STAMP);
      add_tool_icon(editing_mode::scripting, tr("Scripting"), FontNoggit::INFO);
      add_tool_icon(editing_mode::chunk, tr("Chunk Manipulator"), FontNoggit::INFO);
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
