// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_EXTENDEDSLIDER_HPP
#define NOGGIT_EXTENDEDSLIDER_HPP

#include <QWidget>
#include "ui_ExtendedSliderUi.h"

namespace noggit
{
    namespace Red::UiCommon
    {
        class ExtendedSlider : public QWidget
        {
        public:
            ExtendedSlider(QWidget* parent = nullptr);

            void setPrefix(const QString& prefix);
            void setMinimum(double min);
            void setMaximum(double max);
            void setRange(double min, double max);
            void setDecimals(int decimals);
            void setSingleStep(double val);
            void setSliderRange(int min, int max);
            double value();

            void setTabletSupportEnabled(bool state);

        private:
            Ui::ExtendedSliderUi _ui;
            bool _is_tablet_supported = true;
            bool _is_tablet_affecting = false;
            QWidget* _tablet_popup;
            unsigned _tablet_sens_factor = 0;

        };
    }
}

#endif //NOGGIT_EXTENDEDSLIDER_HPP
