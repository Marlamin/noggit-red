#include "Tool.hpp"
#include <noggit/World.h>
#include <noggit/bool_toggle_property.hpp>
#include <QSignalBlocker>

using namespace noggit::Red::StampMode::Ui;

auto Tool::setPixmap(QPixmap const* pixmap) -> void
{
  _curPixmap = pixmap;
  _label.setPixmap(pixmap->scaled(128, 128));
}

Tool::Tool(bool_toggle_property* showPalette, float* cursorRotation, QWidget* parent)
: QWidget{parent}, _cursorRotation{cursorRotation}, _radiusOuter{25.f}, _radiusInner{10.f}, _layout{this}
, _label{this}, _btnPalette{"Open Palette", this}
, _sliderRadiusOuter{Qt::Orientation::Horizontal, this}, _spinboxRadiusOuter{this}
, _sliderRadiusInner{Qt::Orientation::Horizontal, this}, _spinboxRadiusInner{this}
, _dialRotation{this}, _curPixmap{nullptr}
{
  _layout.addRow(&_label);
  _layout.addRow(&_btnPalette);
  _layout.setAlignment(&_label, Qt::AlignHCenter);
  _layout.setAlignment(&_btnPalette, Qt::AlignHCenter);
  _sliderRadiusOuter.setRange(.1f, 1000.f);
  _sliderRadiusOuter.setValue(_radiusOuter);
  _spinboxRadiusOuter.setRange(.1f, 1000.f);
  _spinboxRadiusOuter.setDecimals(1);
  _spinboxRadiusOuter.setValue(_radiusOuter);
  _sliderRadiusInner.setRange(.1f, 1000.f);
  _sliderRadiusInner.setValue(_radiusInner);
  _spinboxRadiusInner.setRange(.1f, 1000.f);
  _spinboxRadiusInner.setDecimals(1);
  _spinboxRadiusInner.setValue(_radiusInner);
  _dialRotation.setRange(0, 360);
  _dialRotation.setWrapping(true);
  _dialRotation.setSingleStep(10);
  _layout.addRow("Outer Radius:", &_spinboxRadiusOuter);
  _layout.addRow(&_sliderRadiusOuter);
  _layout.addRow("Inner Radius:", &_spinboxRadiusInner);
  _layout.addRow(&_sliderRadiusInner);
  _layout.addRow(&_dialRotation);
  connect(&_btnPalette, &QPushButton::pressed, [showPalette](void) -> void { showPalette->toggle(); });
  connect(&_sliderRadiusOuter, &QSlider::valueChanged, [this](int val) -> void
  {
    if(val < _radiusInner)
    {
      QSignalBlocker blocker{&_sliderRadiusOuter};
      _sliderRadiusOuter.setValue(_radiusInner);
      return;
    }

    _radiusOuter = val;
    QSignalBlocker blocker{&_spinboxRadiusOuter};
    _spinboxRadiusOuter.setValue(_radiusOuter);
  });
  connect(&_spinboxRadiusOuter, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double val) -> void
  {
    if(val < _radiusInner)
    {
      QSignalBlocker blocker{&_spinboxRadiusOuter};
      _spinboxRadiusOuter.setValue(_radiusInner);
      return;
    }

    _radiusOuter = val;
    QSignalBlocker blocker{&_sliderRadiusOuter};
    _sliderRadiusOuter.setValue(_radiusOuter);
  });
  connect(&_sliderRadiusInner, &QSlider::valueChanged, [this](int val) -> void
  {
    if(val > _radiusOuter)
    {
      QSignalBlocker blocker{&_sliderRadiusInner};
      _sliderRadiusInner.setValue(_radiusOuter);
      return;
    }

    _radiusInner = val;
    QSignalBlocker blocker{&_spinboxRadiusInner};
    _spinboxRadiusInner.setValue(_radiusInner);
  });
  connect(&_spinboxRadiusInner, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double val) -> void
  {
    if(val > _radiusOuter)
    {
      QSignalBlocker blocker{&_spinboxRadiusInner};
      _spinboxRadiusInner.setValue(_radiusOuter);
      return;
    }

    _radiusInner = val;
    QSignalBlocker blocker{&_sliderRadiusInner};
    _sliderRadiusInner.setValue(val);
  });
  connect(&_dialRotation, &QDial::valueChanged, [this](int val) -> void
  {
    _rotation = val;
    *_cursorRotation = _rotation / 360.0f;
  });
  _dialRotation.setValue(0);
}

auto Tool::getOuterRadius(void) const -> float
{
  return _radiusOuter;
}

auto Tool::getInnerRadius(void) const -> float
{
  return _radiusInner;
}

auto Tool::stamp(World* world, math::vector_3d const& pos, float dt, bool doAdd) const -> void
{
  if(!_curPixmap)
    return;
  
  world->stamp(pos, dt, doAdd, _curPixmap, _radiusOuter, _radiusInner, _rotation);
}
