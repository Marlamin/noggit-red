// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TABLETMANAGER_HPP
#define NOGGIT_TABLETMANAGER_HPP

#include <QObject>

namespace Noggit
{
    class TabletManager : public QObject
    {
    Q_OBJECT
    public:
        static TabletManager* instance()
        {
          static TabletManager inst;
          return &inst;
        }

        void setIsActive(bool state) { _is_active = state; };
        bool isActive() { return _is_active; };
        double pressure() { return _pressure; };
        void setPressure(double pressure) { _pressure = pressure; emit pressureChanged(pressure); };

    signals:
      void pressureChanged(double pressure);


    private:
        TabletManager() : QObject() {}

        // gets activated by MapView::tabletEvent(QTabletEvent* event)
        bool _is_active = false;
        // 0.0 to 1.0 scale
        double _pressure = 0.0;

    };

}

#endif //NOGGIT_TABLETMANAGER_HPP
