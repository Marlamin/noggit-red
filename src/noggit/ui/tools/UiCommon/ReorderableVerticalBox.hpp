// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_REORDERABLEVERTICALBOX_HPP
#define NOGGIT_REORDERABLEVERTICALBOX_HPP

#include <QWidget>
#include <QPoint>

namespace Noggit::Ui::Tools
{
  class ReorderableVerticalBox : public QWidget
  {
  public:

    ReorderableVerticalBox(QWidget* parent = nullptr);;

    void mouseMoveEvent(QMouseEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

    bool IsMinimumDistanceRiched(QMouseEvent* event);

    void paintEvent(QPaintEvent*) override;

    void mouseReleaseEvent(QMouseEvent*) override;

    void setActiveRectWidget(QWidget* widget);

  private:
    int oldX;
    int oldY;
    int mouseClickX;
    int mouseClickY;
    QPoint dragStartPosition;
    bool dragInitiated = false;
    QWidget* activeRectWidget;
  };
}

#endif //NOGGIT_REORDERABLEVERTICALBOX_HPP
