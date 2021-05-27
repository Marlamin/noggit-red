// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>

#include <functional>
#include <string>

namespace noggit
{
  namespace ui
  {
    class hole_tool : public QWidget
    {
    Q_OBJECT

    public:
      hole_tool(QWidget* parent = nullptr);

      void changeRadius(float change);

      float brushRadius() const { return _radius; }

      void setRadius(float radius);

    private:

      QSlider* _radius_slider;
      QDoubleSpinBox* _radius_spin;

      float _radius = 15.0f;

    };
  }
}

