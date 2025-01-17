// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>

#include <string>

class QLabel;

namespace Noggit
{
  namespace Ui
  {
    class detail_infos : public widget
    {
    private:
      QLabel* _info_text;

    public:
      detail_infos(QWidget* parent);
      void setText (const std::string& t);
      virtual QSize sizeHint() const override { return QSize(350, 250); };
    };
  }
}
