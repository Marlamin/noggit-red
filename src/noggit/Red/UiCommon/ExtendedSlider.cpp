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
, _tablet_manager(TabletManager::instance())
{
  _ui.setupUi(this);

  // popup
  _tablet_popup = new QWidget(this);
  auto layout = new QVBoxLayout(_tablet_popup);

  auto tablet_enabled = new QCheckBox(_tablet_popup);
  tablet_enabled->setText("Use Tablet");
  layout->addWidget(tablet_enabled);
  _ui.tabletControlMenuButton->setIcon(font_awesome_icon(font_awesome::icons::pen));

  connect(tablet_enabled, &QCheckBox::stateChanged,
          [=](int state)
          {
            _is_tablet_affecting = state;
            _ui.tabletControlMenuButton->setIcon(font_awesome_icon(
                state ? font_awesome::icons::edit : font_awesome::icons::pen));
          });

  layout->addWidget(new QLabel("Sensitivity:", _tablet_popup));
  auto sens_slider = new QSlider(_tablet_popup);
  sens_slider->setRange(0, 1000);
  sens_slider->setValue(300);
  sens_slider->setOrientation(Qt::Horizontal);
  sens_slider->setSingleStep(1);
  sens_slider->setMinimumWidth(200);

  auto sens_spin = new QDoubleSpinBox(_tablet_popup);
  sens_spin->setRange(0, 1000);
  sens_spin->setValue(300);
  sens_spin->setDecimals(2);

  auto sens_slider_panel = new QWidget(_tablet_popup);
  auto sens_slider_panel_layout = new QHBoxLayout(sens_slider_panel);
  sens_slider_panel_layout->addWidget(sens_slider);
  sens_slider_panel_layout->addWidget(sens_spin);
  layout->addWidget(sens_slider_panel);

  _tablet_popup->updateGeometry();
  _tablet_popup->adjustSize();
  _tablet_popup->update();
  _tablet_popup->repaint();

  connect(sens_slider, &QSlider::valueChanged,
          [=](int value)
          {
            const QSignalBlocker blocker(sens_spin);
            sens_spin->setValue(static_cast<double>(value));
            _tablet_sens_factor = value;
          });

  connect(sens_spin, qOverload<double>(&QDoubleSpinBox::valueChanged),
          [=](double value)
          {
              const QSignalBlocker blocker(sens_slider);
              sens_slider->setValue(static_cast<int>(value));
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
          [=](int v)
          {
            const QSignalBlocker blocker(_ui.doubleSpinBox);

            double spin_value = (_ui.doubleSpinBox->maximum() - _ui.doubleSpinBox->minimum())
            * (v / static_cast<float>((_ui.slider->maximum() - _ui.slider->minimum())));

            _ui.doubleSpinBox->setValue(spin_value);

            emit valueChanged(value());
          });

  connect(_ui.doubleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
          [=](double v)
          {
              const QSignalBlocker blocker(_ui.slider);

              int slider_value = (_ui.slider->maximum() - _ui.slider->minimum())
                                  * (_ui.doubleSpinBox->value() / (_ui.doubleSpinBox->maximum() - _ui.doubleSpinBox->minimum()));

              _ui.slider->setValue(slider_value);
              emit valueChanged(value());
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

  if (_is_tablet_supported && _is_tablet_affecting && _tablet_manager->isActive())
  {
    return std::min(_ui.doubleSpinBox->maximum(), _ui.doubleSpinBox->value()
    + (_ui.doubleSpinBox->maximum() - _ui.doubleSpinBox->minimum()) * _tablet_manager->pressure() * (_tablet_sens_factor / 1000.0));
  }

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

void ExtendedSlider::setValue(double value)
{
  _ui.doubleSpinBox->setValue(value);
}
