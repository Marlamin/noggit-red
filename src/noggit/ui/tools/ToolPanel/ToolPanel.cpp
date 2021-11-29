// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ToolPanel.hpp"
#include <QScrollBar>

using namespace noggit::ui::tools;

ToolPanel::ToolPanel(QWidget* parent)
: QDockWidget(parent)
{
  auto body = new QWidget(this);
  _ui.setupUi(body);
  setWidget(body);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  layout()->setAlignment(Qt::AlignTop);
  setFixedWidth(250 + 15);
}

void ToolPanel::setCurrentIndex(int index)
{
  for (auto& widget : _widgets)
    widget->setVisible(false);

  _widgets.at(index)->setVisible(true);
  _ui.scrollAreaWidgetContents->adjustSize();
}

void ToolPanel::registerTool(QString const& title, QWidget* widget)
{
  _widgets.push_back(widget);
  _ui.scrollAreaWidgetContents->layout()->addWidget(widget);
  _titles.emplace_back(title);
}