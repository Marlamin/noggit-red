// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ActionManager.hpp"
#include <noggit/MapView.h>

using namespace noggit;

std::deque<Action*>* ActionManager::getActionStack()
{
  return &_action_stack;
}

Action* ActionManager::getCurrentAction() const
{
  return _cur_action;
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
}

Action* ActionManager::beginAction(MapView* map_view
                                   , int flags
                                   , int modality_controls)
{
  if (_cur_action)
    return _cur_action;

  // clean canceled actions
  if (_undo_index)
  {
    for (int i = 0; i < _undo_index; ++i)
    {
      delete _action_stack.back();
      _action_stack.pop_back();
    }
    _undo_index = 0;
  }

  // prevent undo stack overflow
  if (_action_stack.size() == _limit)
  {
    Action* old_action = _action_stack.front();
    delete old_action;
    _action_stack.pop_front();
  }

  auto action = new Action(map_view);
  _action_stack.push_back(action);

  action->setFlags(flags);
  action->setModalityControllers(modality_controls);

  _cur_action = action;

  return action;
}

void ActionManager::endAction()
{
  assert(_cur_action && "ActionStack Error: endAction() called with no action running.");

  _cur_action->finish();
  _cur_action = nullptr;
}

void ActionManager::endActionOnModalityMismatch(unsigned modality_controls)
{
  if (!modality_controls)
    return;

  if (!_cur_action)
    return;

  if ((modality_controls & _cur_action->getModalityControllers()) != _cur_action->getModalityControllers())
  {
    _cur_action->finish();
    _cur_action = nullptr;
  }
}

void ActionManager::undo()
{
  assert(!_cur_action && "ActionStack Error: undo initiated while action is running.");

  if (_action_stack.empty())
    return;

  int index = _action_stack.size() - _undo_index - 1;

  if (index < 0)
    return;

  Action* action = _action_stack.at(index);
  action->undo();

  _undo_index++;
}

void ActionManager::redo()
{
  assert(!_cur_action && "ActionStack Error: redo initiated while action is running.");

  if (_action_stack.empty())
    return;

  if (!_undo_index)
    return;

  unsigned index = _action_stack.size() - _undo_index;

  Action* action = _action_stack.at(index);
  action->undo(true);

  _undo_index--;
}

ActionManager::~ActionManager()
{
  for (auto& action : _action_stack)
  {
    delete action;
  }

  _action_stack.clear();
}