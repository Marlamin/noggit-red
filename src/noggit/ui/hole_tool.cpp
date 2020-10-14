// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include "hole_tool.hpp"
#include <cmath>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>

namespace noggit
{
  namespace ui
  {
    hole_tool::hole_tool(QWidget* parent) : QWidget(parent)
    {
      auto layout = new QFormLayout(this);

      _radius_spin = new QDoubleSpinBox (this);
      _radius_spin->setRange (0.0f, 250.0f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);

      layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _radius_slider->setRange (0, 250);
      _radius_slider->setSliderPosition (_radius);

      layout->addRow (_radius_slider);

      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
          , [&] (double v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_slider);
                  _radius_slider->setSliderPosition ((int)std::round (v));
                }
      );

      connect ( _radius_slider, &QSlider::valueChanged
          , [&] (int v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_spin);
                  _radius_spin->setValue(v);
                }
      );
    }

    void hole_tool::changeRadius(float change)
    {
      _radius_spin->setValue (_radius + change);
    }
  }
}
