// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "StackedWidget.hpp"

#include <QVBoxLayout>

StackedWidget::StackedWidget(QWidget * parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , auto_resize{false}
  , curr_index(0)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
}

int StackedWidget::count()
{
  return widgets.count();
}

void StackedWidget::addWidget(QWidget* w)
{
  widgets.append(w);
  layout()->addWidget(w);
  showCurrentWidget();
}

void StackedWidget::removeLast()
{
  QWidget* last = widgets.last();
  widgets.removeLast();
  layout()->removeWidget(last);
  showCurrentWidget();

  curr_index = 0;
}

QWidget * StackedWidget::currentWidget()
{ return widgets.at(curr_index); }

void StackedWidget::setCurrentIndex(int i)
{
  curr_index = i;
  showCurrentWidget();
}

void StackedWidget::showCurrentWidget()
{
  if (widgets.count() > 0)
  {
      foreach (QWidget * widget, widgets)
        widget->hide();

    widgets.at(curr_index)->show();
    updateGeometry();
  }
}

QSize StackedWidget::sizeHint()
{
  if (auto_resize
      && count() > 0)
    return currentWidget()->minimumSize();
  else
    return QWidget::sizeHint();
}
