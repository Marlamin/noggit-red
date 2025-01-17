// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_ACTIONHISTORYNAVIGATOR_HPP
#define NOGGIT_ACTIONHISTORYNAVIGATOR_HPP

#include <QWidget>

class QListWidget;
class QButtonGroup;
class QLabel;

namespace Noggit
{
  class Action;

  namespace Ui::Tools
  {
    class ActionHistoryNavigator : public QWidget
    {
      Q_OBJECT
    public:
      ActionHistoryNavigator(QWidget* parent = nullptr);

    public slots:
      void pushAction(Noggit::Action* action);
      void popFront();
      void popBack();
      void purge();
      void changeCurrentAction(unsigned index);

    signals:
      void currentActionChanged(unsigned index);

    private:
      QListWidget* _action_stack;
      QButtonGroup* _active_action_button_group;
      QLabel* _stack_size_label;

    private:
        void updateStackSizeLabel();

    };
  }
}

#endif //NOGGIT_ACTIONHISTORYNAVIGATOR_HPP
