// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ExtendedSlider.hpp"
#include <noggit/ui/font_awesome.hpp>
#include <cfloat>

#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalBlocker>

using namespace noggit::Red::UiCommon;
using namespace noggit::ui;

ExtendedSlider::ExtendedSlider(QWidget* parent)
: QWidget(parent)
{
  _ui.setupUi(this);

  // popup
  _tablet_popup = new QWidget(this);
  auto layout = new QVBoxLayout(_tablet_popup);

  auto tablet_enabled = new QCheckBox(_tablet_popup);
  tablet_enabled->setText("Use Tablet");
  layout->addWidget(tablet_enabled);

  connect(tablet_enabled, &QCheckBox::stateChanged,
          [=](int state)
          {
            _is_tablet_affecting = state;
            _ui.tabletControlMenuButton->setIcon(font_awesome_icon(
                state ? font_awesome::icons::edit : font_awesome::icons::pen));
          });

  layout->addWidget(new QLabel("Sensitivity:", _tablet_popup));
  auto sens_slider = new QSlider(_tablet_popup);
  sens_slider->setRange(0, 100);
  sens_slider->setOrientation(Qt::Horizontal);
  layout->addWidget(sens_slider);

  connect(sens_slider, &QSlider::valueChanged,
          [=](int value)
          {
            _tablet_sens_factor = value;
          });

  _tablet_popup->setVisible(false);

  // ui
  connect(_ui.tabletControlMenuButton, &QPushButton::clicked,
          [=]()
          {
            QPoint new_pos = mapToGlobal(
                QPoint(_ui.tabletControlMenuButton->pos().x() - _tablet_popup->width() - 12,
                  _ui.tabletControlMenuButton->pos().y()));

            _tablet_popup->setGeometry(new_pos.x(),
                                       new_pos.y(),
                                       _tablet_popup->width(),
                                       _tablet_popup->height());

            _tablet_popup->setWindowFlags(Qt::Popup);
            _tablet_popup->show();
          });

  _ui.slider->setRange(0, 100);
  connect(_ui.slider, &QSlider::valueChanged,
          [=](int value)
          {
            const QSignalBlocker blocker(_ui.doubleSpinBox);

            double spin_value = (_ui.doubleSpinBox->maximum() - _ui.doubleSpinBox->minimum())
            * (_ui.slider->value() / (_ui.slider->maximum() - _ui.slider->minimum()));

            _ui.doubleSpinBox->setValue(value);
          });

  connect(_ui.doubleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          [=](double value)
          {
              const QSignalBlocker blocker(_ui.slider);

              int slider_value = (_ui.slider->maximum() - _ui.slider->minimum())
                                  * (_ui.doubleSpinBox->value() / (_ui.doubleSpinBox->maximum() - _ui.doubleSpinBox->minimum()));

              _ui.slider->setValue(value);
          });
}

void ExtendedSlider::setMinimum(double min)
{
  _ui.doubleSpinBox->setMinimum(min);
}

void ExtendedSlider::setMaximum(double max)
{
  _ui.doubleSpinBox->setMaximum(max);
}

void ExtendedSlider::setRange(double min, double max)
{
  _ui.doubleSpinBox->setMinimum(min);
  _ui.doubleSpinBox->setMaximum(max);
}

void ExtendedSlider::setDecimals(int decimals)
{
  Q_ASSERT(decimals <= DBL_MAX_10_EXP + DBL_DIG && decimals >= 0);
  _ui.doubleSpinBox->setDecimals(decimals);

}

void ExtendedSlider::setSingleStep(double val)
{
  Q_ASSERT(val > 0.0);
  _ui.doubleSpinBox->setSingleStep(val);
}


double ExtendedSlider::value()
{
  // TODO: pressure stuff here
  return _ui.doubleSpinBox->value();
}

void ExtendedSlider::setPrefix(const QString& prefix)
{
  _ui.label->setText(prefix);
}

void ExtendedSlider::setTabletSupportEnabled(bool state)
{
  _is_tablet_supported = state;
  _ui.tabletControlMenuButton->setVisible(state);
}

void ExtendedSlider::setSliderRange(int min, int max)
{
  _ui.slider->setRange(min, max);
}