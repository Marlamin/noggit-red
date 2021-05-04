// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ToolPanel.hpp"
#include <QScrollArea>
#include <QScrollBar>

using namespace noggit::Red;

ToolPanel::ToolPanel(QWidget* parent)
: QDockWidget(parent)
{
  auto body = new QWidget(this);
  _ui.setupUi(body);
  setWidget(body);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  setFixedWidth(250 + 15);
}

void ToolPanel::setCurrentIndex(int index)
{
  _ui.toolPanelStack->setCurrentIndex(index);
  setWindowTitle(_titles.at(index));

  auto widget_min_width = _ui.toolPanelStack->currentWidget()->minimumWidth();
  setFixedWidth(_ui.scrollArea->verticalScrollBar()->isVisible()
  ? widget_min_width + _ui.scrollArea->verticalScrollBar()->width()
  : widget_min_width);
}

void ToolPanel::registerTool(QString const& title, QWidget* widget)
{
  _ui.toolPanelStack->addWidget(widget);
  _titles.emplace_back(title);
}