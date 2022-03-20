// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/DetailInfos.h>

#include <QVBoxLayout>
#include <QScrollArea>

namespace Noggit
{
  namespace Ui
  {
    detail_infos::detail_infos(QWidget* parent)
      : widget (parent, Qt::Window)
    {
      setWindowFlags (Qt::Tool);
      auto layout (new QVBoxLayout (this));

      auto scroll_area = new QScrollArea(this);
      scroll_area->setWidget(_info_text = new QLabel(this));
      scroll_area->setWidgetResizable(true);

      _info_text->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
      _info_text->setTextInteractionFlags(Qt::TextSelectableByMouse);
      _info_text->setAlignment(Qt::AlignTop);
      _info_text->setTextFormat(Qt::RichText);
      layout->addWidget(scroll_area);
    }

    void detail_infos::setText (const std::string& t)
    {
      _info_text->setText (t.c_str ());
    }
  }
}
