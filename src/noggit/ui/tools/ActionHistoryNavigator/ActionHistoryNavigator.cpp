// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ActionHistoryNavigator.hpp"
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>
#include <noggit/ui/font_awesome.hpp>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QDateTime>
#include <QLabel>

using namespace noggit::ui::tools;
using namespace noggit::ui;

ActionHistoryNavigator::ActionHistoryNavigator(QWidget* parent)
: QWidget(parent)
{
  _active_action_button_group = new QButtonGroup(this);

  auto layout = new QVBoxLayout(this);
  _action_stack = new QListWidget(this);
  layout->addWidget(_action_stack);
  _action_stack->setSelectionMode(QAbstractItemView::SingleSelection);
  _action_stack->setSelectionBehavior(QAbstractItemView::SelectRows);
  _action_stack->setSelectionRectVisible(true);

  auto action_mgr = NOGGIT_ACTION_MGR;

  connect(action_mgr, &noggit::ActionManager::popBack, this, &ActionHistoryNavigator::popBack);
  connect(action_mgr, &noggit::ActionManager::popFront, this, &ActionHistoryNavigator::popFront);
  connect(action_mgr, &noggit::ActionManager::addedAction, this, &ActionHistoryNavigator::pushAction);
  connect(action_mgr, &noggit::ActionManager::purged, this, &ActionHistoryNavigator::purge);
  connect(action_mgr, &noggit::ActionManager::currentActionChanged, this, &ActionHistoryNavigator::changeCurrentAction);

  connect(_active_action_button_group, &QButtonGroup::idClicked
          , [=](int index)
          {
            QSignalBlocker const blocker(action_mgr);
            emit currentActionChanged((_action_stack->count() - (_action_stack->count() - index)) - 1);
            action_mgr->setCurrentAction((_action_stack->count() - index) - 1);
          });

}

void ActionHistoryNavigator::pushAction(noggit::Action* action)
{
  auto item = new QListWidgetItem();
  _action_stack->insertItem(_action_stack->count(), item);

  auto widget = new QWidget();
  widget->setMinimumWidth(150);
  widget->setMinimumHeight(17);
  auto layout = new QHBoxLayout(widget);
  layout->setContentsMargins(1, 1, 1, 1);
  layout->setAlignment(Qt::AlignLeft);

  auto radio = new QRadioButton(widget);
  _active_action_button_group->addButton(radio);
  _active_action_button_group->setId(radio, _action_stack->count() - 1);
  layout->addWidget(radio);

  QDateTime date = QDateTime::currentDateTime();
  layout->addWidget(new QLabel(date.toString("hh:mm:ss ap"), widget));
  layout->addStretch();

  _action_stack->setItemWidget(item, widget);
}

void ActionHistoryNavigator::popFront()
{

  auto radio = static_cast<QRadioButton*>(static_cast<QHBoxLayout*>(_action_stack->itemWidget(
    _action_stack->item(0))->layout())->itemAt(0)->widget());
  _active_action_button_group->removeButton(radio);
  delete radio;

  _action_stack->removeItemWidget(_action_stack->item(0));
  auto item = _action_stack->takeItem(0);
  delete item;
}

void ActionHistoryNavigator::popBack()
{
  auto radio = static_cast<QRadioButton*>(static_cast<QHBoxLayout*>(_action_stack->itemWidget(
    _action_stack->item(_action_stack->count() - 1))->layout())->itemAt(0)->widget());
  _active_action_button_group->removeButton(radio);
  delete radio;

  _action_stack->removeItemWidget(_action_stack->item(_action_stack->count() - 1));
  auto item = _action_stack->takeItem(_action_stack->count() - 1);
  delete item;
}

void ActionHistoryNavigator::purge()
{
  for (int i = 0; i < _action_stack->count(); ++i)
  {
    auto radio = static_cast<QRadioButton*>(static_cast<QHBoxLayout*>(_action_stack->itemWidget(
      _action_stack->item(i))->layout())->itemAt(0)->widget());
    _active_action_button_group->removeButton(radio);
    delete radio;
  }
  _action_stack->clear();

}

void ActionHistoryNavigator::changeCurrentAction(unsigned index)
{
  QSignalBlocker const _(_action_stack);

  int idx = _action_stack->count() - 1 - index;
  if (idx >= 0)
  {
    static_cast<QRadioButton*>(static_cast<QHBoxLayout*>(_action_stack->itemWidget(
      _action_stack->item(idx))->layout())->itemAt(0)->widget())->setChecked(true);
  }
  else if (_action_stack->count())
  {
    _active_action_button_group->setExclusive(false);
    static_cast<QRadioButton*>(static_cast<QHBoxLayout*>(_action_stack->itemWidget(
      _action_stack->item(0))->layout())->itemAt(0)->widget())->setChecked(false);
    _active_action_button_group->setExclusive(true);
  }

  //_action_stack->setCurrentItem(_action_stack->item(_action_stack->count() - 1 - index));
}