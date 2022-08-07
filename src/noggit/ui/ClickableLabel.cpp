// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ClickableLabel.hpp>

namespace Noggit
{
  namespace Ui
  {
    ClickableLabel::ClickableLabel(QWidget * parent) : QLabel(parent){}

    void ClickableLabel::mouseReleaseEvent (QMouseEvent* event)
    {
        emit clicked();

        if (event->button() == Qt::MiddleButton)
            emit middleClicked();

        if (event->button() == Qt::LeftButton)
            emit leftClicked();

        if (event->button() == Qt::RightButton)
            emit rightClicked();
    }
  }
}
