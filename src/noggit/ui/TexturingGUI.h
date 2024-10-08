// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/TextureManager.h>
#include <noggit/ui/widget.hpp>
#include <optional>

class QMouseEvent;
class QListView;

namespace Noggit
{
  namespace Ui
  {
    class current_texture;

    struct tileset_chooser : public widget
    {
      Q_OBJECT

    public:
      tileset_chooser (QWidget* parent = nullptr);

    signals:
      void selected (std::string);

    };

    class selected_texture
    {
    public:
      static void set(scoped_blp_texture_reference t);
      static std::optional<scoped_blp_texture_reference> get();
      static std::optional<scoped_blp_texture_reference> texture;
    };
  }
}
