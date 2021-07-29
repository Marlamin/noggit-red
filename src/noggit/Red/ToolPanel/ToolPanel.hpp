// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TOOLPANEL_HPP
#define NOGGIT_TOOLPANEL_HPP

#include <QWidget>
#include <QDockWidget>
#include <ui_ToolPanelScroll.h>

#include <vector>

namespace noggit
{
  namespace Red
  {
    class ToolPanel : public QDockWidget
    {
      Q_OBJECT

    public:
      explicit ToolPanel(QWidget* parent = nullptr);

      void setCurrentIndex(int index);
      void registerTool(QString const& title, QWidget* widget);

    private:
      Ui::toolPanel _ui;
      std::vector<QString> _titles;
      std::vector<QWidget*> _widgets;

    };
  }
}

#endif //NOGGIT_TOOLPANEL_HPP
