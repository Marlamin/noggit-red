// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/BoolToggleProperty.hpp>

#include <QtWidgets/QCheckBox>

namespace Noggit
{
  namespace Ui
  {
    class CheckBox : public QCheckBox
    {
    public:
      CheckBox ( QString label
               , BoolToggleProperty* prop
               , QWidget* parent = nullptr
               )
        : QCheckBox (label, parent)
      {
        connect ( this, &QCheckBox::toggled
                , prop, &BoolToggleProperty::set
                );
        connect ( prop, &BoolToggleProperty::changed
                , this, &QCheckBox::setChecked
                );
        setChecked (prop->get());
      }
    };
  }
}
