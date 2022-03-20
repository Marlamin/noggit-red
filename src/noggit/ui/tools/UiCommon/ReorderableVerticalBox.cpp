// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ReorderableVerticalBox.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QStyleOption>
#include <QPainter>

using namespace Noggit::Ui::Tools;

void ReorderableVerticalBox::mouseMoveEvent(QMouseEvent* event)
{
  if (!dragInitiated)
    return;

  if (!(event->buttons() & Qt::LeftButton))
    return;

  if (!IsMinimumDistanceRiched(event))
  {
    return;
  }
  int y = event->globalY() - mouseClickY + oldY;
  int BottomBorder = reinterpret_cast<QWidget*>(parent())->geometry().height() - this->geometry().height();
  if(y < 0) y = 0;
  else if(y > BottomBorder) y = BottomBorder;
  move(oldX, y);
}

void ReorderableVerticalBox::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() & Qt::LeftButton)
    dragStartPosition = event->globalPos();

  if (!activeRectWidget->rect().contains(event->pos()))
    return;

  dragInitiated = true;

  oldX = this->geometry().x();
  oldY = this->geometry().y();
  mouseClickX = event->globalX();
  mouseClickY = event->globalY();
}

bool ReorderableVerticalBox::IsMinimumDistanceRiched(QMouseEvent* event)
{
  return (event->globalPos() - dragStartPosition).manhattanLength() >= QApplication::startDragDistance();
}


void ReorderableVerticalBox::paintEvent(QPaintEvent *)
{
  QStyleOption o;
  o.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

void ReorderableVerticalBox::mouseReleaseEvent(QMouseEvent* event)
{
  if (!dragInitiated)
    return;

  QVBoxLayout* myLayout = qobject_cast<QVBoxLayout*>(reinterpret_cast<QWidget*>(parent())->layout());
  int y = geometry().y();

  int idx = 0;
  int cur_height = 0;
  while(idx < myLayout->count())
  {
    cur_height += myLayout->itemAt(idx)->widget()->height();

    if (cur_height > y)
      break;
    else
    {
      idx++;
    }
  }

  myLayout->removeWidget(this);
  myLayout->insertWidget(idx , this);

  update();
  myLayout->update();
  this->saveGeometry();

  dragInitiated = false;
}

void ReorderableVerticalBox::setActiveRectWidget(QWidget* widget)
{
  activeRectWidget = widget;
}
