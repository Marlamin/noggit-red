// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ActionManager.hpp"
#include <noggit/MapView.h>
#include <cmath>

using namespace Noggit;

std::deque<Action*>* ActionManager::getActionStack()
{
  return &_action_stack;
}

Action* ActionManager::getCurrentAction() const
{
  return _cur_action;
}

void ActionManager::setCurrentAction(unsigned index)
{
  int index_diff = index - _undo_index;

  if (!index_diff)
    return;

  if (index_diff > 0)
  {
    for (int i = 0; i < index_diff; ++i)
      undo();
  }
  else
  {
    for (int i = 0; i < std::abs(index_diff); ++i)
      redo();
  }
}


void ActionManager::setLimit(unsigned limit)
{
  _limit = limit;
}

unsigned ActionManager::limit() const
{
  return _limit;
}

void ActionManager::purge()
{
  for (auto& action : _action_stack)
  {
    delete action;
  }
  _action_stack.clear();
  _undo_index = 0;
  emit purged();
}

Action* ActionManager::beginAction(MapView* map_view
                                   , int flags
                                   , int modality_controls)
{
  if (_cur_action)
    return _cur_action;

  if (!(flags & eDO_NOT_WRITE_HISTORY))
  {
    // clean canceled actions
    if (_undo_index)
    {
      for (unsigned i = 0; i < _undo_index; ++i)
      {
        delete _action_stack.back();
        _action_stack.pop_back();
        emit popBack();
      }
      _undo_index = 0;
    }

    // prevent undo stack overflow
    if (_action_stack.size() == _limit)
    {
      Action* old_action = _action_stack.front();
      delete old_action;
      _action_stack.pop_front();
      emit popFront();
    }
  }

  auto action = new Action(map_view);
  _action_stack.push_back(action);

  action->setFlags(flags);
  action->setModalityControllers(modality_controls);

  _cur_action = action;

  emit onActionBegin(action);

  return action;
}

void ActionManager::endAction()
{
  assert(_cur_action && "ActionStack Error: endAction() called with no action running.");

  _cur_action->finish();
  if (!(_cur_action->getFlags() & eDO_NOT_WRITE_HISTORY))
  {
    emit addedAction(_cur_action);
  }
  else
  {
    _action_stack.pop_back();
  }

  emit onActionEnd(_cur_action);
  _cur_action = nullptr;
  emit currentActionChanged(_undo_index);
}

void ActionManager::endActionOnModalityMismatch(unsigned modality_controls)
{
  if (!_cur_action)
    return;

  if (!_cur_action->getModalityControllers())
    return;

  if ((modality_controls & _cur_action->getModalityControllers()) != _cur_action->getModalityControllers())
  {
    _cur_action->finish();
    if (!(_cur_action->getFlags() & eDO_NOT_WRITE_HISTORY))
    {
      emit addedAction(_cur_action);
    }
    else
    {
      _action_stack.pop_back();
    }
    _cur_action = nullptr;
    emit currentActionChanged(_undo_index);
  }
}

void ActionManager::undo()
{
  assert(!_cur_action && "ActionStack Error: undo initiated while action is running.");

  if (_action_stack.empty())
    return;

  int index = static_cast<int>(_action_stack.size()) - static_cast<int>(_undo_index) - 1;

  if (index < 0)
    return;

  Action* action = _action_stack.at(index);
  action->undo();

  _undo_index++;
  emit currentActionChanged(_undo_index);
}

void ActionManager::redo()
{
  assert(!_cur_action && "ActionStack Error: redo initiated while action is running.");

  if (_action_stack.empty())
    return;

  if (!_undo_index)
    return;

  unsigned index = static_cast<int>(_action_stack.size()) - static_cast<int>(_undo_index);

  Action* action = _action_stack.at(index);
  action->undo(true);

  _undo_index--;
  emit currentActionChanged(_undo_index);
}

ActionManager::~ActionManager()
{
  for (auto& action : _action_stack)
  {
    delete action;
  }

  _action_stack.clear();
}