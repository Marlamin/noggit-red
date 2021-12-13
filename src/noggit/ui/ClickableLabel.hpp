// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QLabel>

namespace Noggit
{
  namespace Ui
  {
    class ClickableLabel : public QLabel
    {
      Q_OBJECT

    public:
      ClickableLabel(QWidget* parent=nullptr);

    signals:
      void clicked();

    protected:
      virtual void mouseReleaseEvent (QMouseEvent* event) override;
    };
  }
}
