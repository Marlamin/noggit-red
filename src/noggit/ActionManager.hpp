// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGITQT_ACTIONMANAGER_HPP
#define NOGGITQT_ACTIONMANAGER_HPP

#include <QObject>
#include <deque>
#include <stdexcept>
#include <noggit/Action.hpp>

class MapView;

namespace noggit
{

    class ActionManager : public QObject
    {
    Q_OBJECT
    public:
        static ActionManager* instance()
        {
          static ActionManager inst;
          return &inst;
        }

        [[nodiscard]]
        std::deque<Action*>* getActionStack();

        [[nodiscard]]
        Action* getCurrentAction() const;

        void setCurrentAction(unsigned index);

        Action* beginAction(MapView* map_view
                            , int flags = ActionFlags::eNO_FLAG
                            , int modality_controls = ActionModalityControllers::eNONE);

        void endAction();

        void endActionOnModalityMismatch(unsigned modality_controls);

        void setLimit(unsigned limit);

        void purge();

        [[nodiscard]]
        unsigned limit() const;

        void undo();
        void redo();

        ~ActionManager() override;

    signals:
      void popBack();
      void popFront();
      void addedAction(Action* action);
      void purged();
      void currentActionChanged(unsigned index);


    private:
        ActionManager() : QObject() {}

        std::deque<Action*> _action_stack;
        unsigned _limit = 30;
        Action* _cur_action = nullptr;
        unsigned _undo_index = 0;

    };

}

#endif //NOGGITQT_ACTIONMANAGER_HPP
