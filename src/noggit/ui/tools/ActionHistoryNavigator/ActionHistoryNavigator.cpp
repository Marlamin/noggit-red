// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ActionHistoryNavigator.hpp"
#include <noggit/ActionManager.hpp>
#include <noggit/Action.hpp>
#include <noggit/ui/FontAwesome.hpp>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QDateTime>
#include <QLabel>

using namespace Noggit::Ui::Tools;
using namespace Noggit::Ui;

ActionHistoryNavigator::ActionHistoryNavigator(QWidget* parent)
: QWidget(parent)
{
  auto action_mgr = NOGGIT_ACTION_MGR;

  _active_action_button_group = new QButtonGroup(this);

  auto layout = new QVBoxLayout(this);

  unsigned int limit = action_mgr->limit();
  _stack_size_label = new QLabel(this);

  layout->addWidget(_stack_size_label);

  _action_stack = new QListWidget(this);
  layout->addWidget(_action_stack);
  _action_stack->setSelectionMode(QAbstractItemView::SingleSelection);
  _action_stack->setSelectionBehavior(QAbstractItemView::SelectRows);
  _action_stack->setSelectionRectVisible(true);
  _action_stack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _action_stack->setResizeMode(QListView::Adjust);


  connect(action_mgr, &Noggit::ActionManager::popBack, this, &ActionHistoryNavigator::popBack);
  connect(action_mgr, &Noggit::ActionManager::popFront, this, &ActionHistoryNavigator::popFront);
  connect(action_mgr, &Noggit::ActionManager::addedAction, this, &ActionHistoryNavigator::pushAction);
  connect(action_mgr, &Noggit::ActionManager::purged, this, &ActionHistoryNavigator::purge);
  connect(action_mgr, &Noggit::ActionManager::currentActionChanged, this, &ActionHistoryNavigator::changeCurrentAction);

  connect(_active_action_button_group, &QButtonGroup::idClicked
          , [=](int index)
          {
            QSignalBlocker const blocker(action_mgr);
            emit currentActionChanged((_action_stack->count() - (_action_stack->count() - index)) - 1);
            action_mgr->setCurrentAction((_action_stack->count() - index) - 1);
          });

  updateStackSizeLabel();
}

void ActionHistoryNavigator::pushAction(Noggit::Action* action)
{
  auto item = new QListWidgetItem();
  _action_stack->insertItem(_action_stack->count(), item);

  auto widget = new QWidget();
  widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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

  if (action->getFlags() & ActionFlags::eCHUNKS_TERRAIN)
  {
    layout->addWidget(new QLabel(" | Terrain", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNKS_AREAID)
  {
      layout->addWidget(new QLabel(" | Area ID", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNKS_HOLES)
  {
      layout->addWidget(new QLabel(" | Holes", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNKS_VERTEX_COLOR)
  {
      layout->addWidget(new QLabel(" | Vertex Color", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNKS_WATER)
  {
      layout->addWidget(new QLabel(" | Liquid", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNKS_TEXTURE)
  {
      layout->addWidget(new QLabel(" | Textures", widget));
  }
  if (action->getFlags() & ActionFlags::eOBJECTS_REMOVED)
  {
      layout->addWidget(new QLabel(" | Objects Removed", widget));
  }
  if (action->getFlags() & ActionFlags::eOBJECTS_ADDED)
  {
      layout->addWidget(new QLabel(" | Objects Added", widget));
  }
  if (action->getFlags() & ActionFlags::eOBJECTS_TRANSFORMED)
  {
      layout->addWidget(new QLabel(" | Objects Transformed", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNKS_FLAGS)
  {
      layout->addWidget(new QLabel(" | Chunk Flags", widget));
  }
  if (action->getFlags() & ActionFlags::eVERTEX_SELECTION)
  {
      layout->addWidget(new QLabel(" | Vertex Selection", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNK_SHADOWS)
  {
      layout->addWidget(new QLabel(" | Shadows", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNK_DOODADS_EXCLUSION)
  {
      layout->addWidget(new QLabel(" | Ground Effects Exclusion", widget));
  }
  if (action->getFlags() & ActionFlags::eCHUNKS_LAYERINFO)
  {
      layout->addWidget(new QLabel(" | Texture Layer Info", widget)); // todo : separate anim & ground effect?
  }

  layout->addStretch();
  _action_stack->setItemWidget(item, widget);

  updateStackSizeLabel();
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

  updateStackSizeLabel();
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

  updateStackSizeLabel();
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

  updateStackSizeLabel();
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
  updateStackSizeLabel();
}

void Noggit::Ui::Tools::ActionHistoryNavigator::updateStackSizeLabel()
{
    auto action_mgr = NOGGIT_ACTION_MGR;
    unsigned int limit = action_mgr->limit();
    auto stack_size = _action_stack->count(); // action_mgr->_action_stack.size()

    QString labelText = QString("Action stack size : %1/%2").arg(stack_size).arg(limit);
    _stack_size_label->setText(labelText);
}
