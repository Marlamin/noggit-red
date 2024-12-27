// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>
#include <QtWidgets/qcombobox.h>
#include <QPushButton>

#include <noggit/World.h>

namespace Noggit
{
    namespace Ui
    {
        class light_object_editor : public widget
        {
        private:
            color_widgets::ColorSelector* ColorPicker;
        public:
            light_object_editor(QWidget* parent, World* world);
            virtual QSize sizeHint() const override { return QSize(560, 500); };
        };
    }
}