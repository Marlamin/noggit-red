// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>
#include <noggit/ui/FontNoggit.hpp>

class QFormLayout;

namespace Noggit
{
  namespace Ui
  {
    class help : public widget
    {
    public:
      help(QWidget* parent = nullptr);

    private:
      inline void generate_hotkey_row(std::initializer_list<FontNoggit::Icons>&& hotkeys, const char* description, QFormLayout* layout);
    };
  }
}
