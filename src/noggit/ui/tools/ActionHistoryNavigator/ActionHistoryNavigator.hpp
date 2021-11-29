// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_ACTIONHISTORYNAVIGATOR_HPP
#define NOGGIT_ACTIONHISTORYNAVIGATOR_HPP

#include <QWidget>
#include <QListWidget>
#include <QButtonGroup>



namespace noggit
{
  class Action;

  namespace ui::tools
  {
    class ActionHistoryNavigator : public QWidget
    {
      Q_OBJECT
    public:
      ActionHistoryNavigator(QWidget* parent = nullptr);

    public slots:
      void pushAction(noggit::Action* action);
      void popFront();
      void popBack();
      void purge();
      void changeCurrentAction(unsigned index);

    signals:
      void currentActionChanged(unsigned index);

    private:
      QListWidget* _action_stack;
      QButtonGroup* _active_action_button_group;


    };
  }
}

#endif //NOGGIT_ACTIONHISTORYNAVIGATOR_HPP
