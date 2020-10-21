// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MinimapCreator.hpp"

#include <noggit/MapView.h>

#include <util/qt/overload.hpp>

#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QTabWidget>


namespace noggit
{
  namespace ui
  {
    MinimapCreator::MinimapCreator (
        MapView* mapView,
        QWidget* parent ) : QWidget(parent)
    {
      auto layout = new QHBoxLayout(this);

      auto layout_left = new QFormLayout (this);

      layout->addItem(layout_left);

      auto settings_tabs = new QTabWidget (this);
      layout->addWidget(settings_tabs);

      // Generate
      auto generate_widget = new QWidget(this);
      auto generate_layout = new QFormLayout(generate_widget);
      settings_tabs->addTab(generate_widget, "Generate");

      _radius_spin = new QDoubleSpinBox (generate_widget);
      _radius_spin->setRange (0.0f, 100.0f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);

      generate_layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, generate_widget);
      _radius_slider->setRange (0, 100);
      _radius_slider->setSliderPosition (_radius);

      generate_layout->addRow (_radius_slider);

      auto cur_adt_btn = new QPushButton("Current ADT", generate_widget);
      auto sel_adts_btn = new QPushButton("Selected ADTs", generate_widget);
      auto all_adts_btn = new QPushButton("Map", generate_widget);

      generate_layout->addRow (cur_adt_btn);
      generate_layout->addRow (sel_adts_btn);
      generate_layout->addRow (all_adts_btn);

      // Render settings box
      auto render_settings_box = new QGroupBox("Render options", generate_widget);
      generate_layout->addRow (render_settings_box);

      auto render_settings_box_layout = new QFormLayout (render_settings_box);

      auto draw_models = new QCheckBox("Draw models", render_settings_box);
      draw_models->setChecked(_render_settings.draw_m2);
      render_settings_box_layout->addRow (draw_models);

      auto draw_wmos = new QCheckBox("Draw WMOs", render_settings_box);
      draw_wmos->setChecked(_render_settings.draw_wmo);
      render_settings_box_layout->addRow (draw_wmos);

      auto draw_water = new QCheckBox("Draw water", render_settings_box);
      draw_water->setChecked(_render_settings.draw_water);
      render_settings_box_layout->addRow (draw_water);

      auto draw_adt = new QCheckBox("Draw ADT grid", render_settings_box);
      draw_adt->setChecked(_render_settings.draw_adt_grid);
      render_settings_box_layout->addRow (draw_adt);

      auto draw_elevation = new QCheckBox("Draw elevation lines", render_settings_box);
      draw_elevation->setChecked(_render_settings.draw_elevation);
      render_settings_box_layout->addRow (draw_elevation);

      // Connections

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

      // Render settings

      connect (draw_models, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.draw_m2 = s;
      });

      connect (draw_wmos, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.draw_wmo = s;
      });

      connect (draw_water, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.draw_water = s;
      });

      connect (draw_adt, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.draw_adt_grid = s;
      });

      connect (draw_elevation, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.draw_elevation = s;
      });

      // Buttons
      connect(cur_adt_btn, &QPushButton::clicked, [=]() {
        _render_settings.export_mode = MinimapGenMode::CURRENT_ADT;
        mapView->initMinimapSave();
      });

      connect(sel_adts_btn, &QPushButton::clicked, [=]() {
        _render_settings.export_mode = MinimapGenMode::SELECTED_ADTS;
        mapView->initMinimapSave();
      });

      connect(all_adts_btn, &QPushButton::clicked, [=]() {
        _render_settings.export_mode = MinimapGenMode::MAP;
        mapView->initMinimapSave();
      });

      setMinimumWidth(sizeHint().width());
    }

    void MinimapCreator::changeRadius(float change)
    {
      _radius_spin->setValue (_radius + change);
    }

    QSize MinimapCreator::sizeHint() const
    {
      return QSize(215, height());
    }
  }
}
