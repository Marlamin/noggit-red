// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_STACKEDWIDGET_HPP
#define NOGGIT_STACKEDWIDGET_HPP

#include <QList>
#include <QWidget>

class StackedWidget : public QWidget
{
Q_OBJECT

public:
  StackedWidget(QWidget * = 0, Qt::WindowFlags = 0);

  void addWidget(QWidget *);
  void removeLast();
  int count();
  QWidget * currentWidget();

  void setAutoResize(bool yes)
  { auto_resize = yes; }

  QSize sizeHint();

protected:
  void showCurrentWidget();

private:
  bool auto_resize;
  int curr_index;
  QList<QWidget *> widgets;

public slots:
  void setCurrentIndex(int);
};

#endif //NOGGIT_STACKEDWIDGET_HPP
