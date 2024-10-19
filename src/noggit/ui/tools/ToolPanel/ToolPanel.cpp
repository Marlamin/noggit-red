// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Tool.hpp>

#include "ToolPanel.hpp"
#include <QScrollBar>

using namespace Noggit::Ui::Tools;

ToolPanel::ToolPanel(QWidget* parent)
  : QDockWidget(parent)
{
  auto body = new QWidget(this);
  _ui.setupUi(body);
  setWidget(body);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  layout()->setAlignment(Qt::AlignTop);
  setMinimumWidth(250 + 15);
}

void ToolPanel::setCurrentTool(editing_mode mode)
{
  for (auto&& [tool, widget] : _tools)
  {
    if (tool->editingMode() == mode)
    {
      widget->setVisible(true);
    }
    else
    {
      widget->setVisible(false);
    }
  }

  _ui.scrollAreaWidgetContents->adjustSize();
}

void ToolPanel::registerTool(Tool* tool, QWidget* widget)
{
  _ui.scrollAreaWidgetContents->layout()->addWidget(widget);
  _tools.emplace_back(std::make_pair(tool, widget));
}
