// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TOOLPANEL_HPP
#define NOGGIT_TOOLPANEL_HPP

#include <noggit/tool_enums.hpp>

#include <QWidget>
#include <QDockWidget>
#include <ui_ToolPanelScroll.h>

#include <vector>
#include <utility>

namespace Noggit
{
  class Tool;

  namespace Ui::Tools
  {
    class ToolPanel : public QDockWidget
    {
      Q_OBJECT

    public:
      explicit ToolPanel(QWidget* parent = nullptr);

      void setCurrentTool(editing_mode mode);
      void registerTool(Tool* tool, QWidget* widget);

    private:
      ::Ui::toolPanel _ui;
      std::vector<std::pair<Tool*, QWidget*>> _tools;

    };
  }
}

#endif //NOGGIT_TOOLPANEL_HPP
