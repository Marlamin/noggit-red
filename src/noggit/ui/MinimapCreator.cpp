// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MinimapCreator.hpp"

#include <noggit/MapView.h>
#include <noggit/World.h>

#include <util/qt/overload.hpp>

#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QTabWidget>
#include <QScrollArea>
#include <QWheelEvent>
#include <QApplication>
#include <QComboBox>
#include <QProgressBar>


namespace noggit
{
  namespace ui
  {
    MinimapCreator::MinimapCreator (
        MapView* mapView,
        World* world,
        QWidget* parent ) : QWidget(parent)
    {
      setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      auto layout = new QHBoxLayout(this);

      // Left side

      auto layout_left = new QFormLayout (this);
      layout->addLayout(layout_left);

      auto scroll_minimap = new QScrollArea(this);
      scroll_minimap->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

      _minimap_widget = new minimap_widget(this);
      layout_left->addWidget(scroll_minimap);

      scroll_minimap->setAlignment(Qt::AlignCenter);
      scroll_minimap->setWidget(_minimap_widget);
      scroll_minimap->setWidgetResizable(true);
      scroll_minimap->setFixedSize(QSize(512, 512));


      _minimap_widget->world(world);
      _minimap_widget->draw_boundaries(true);
      _minimap_widget->use_selection(&_render_settings.selected_tiles);
      _minimap_widget->camera(mapView->getCamera());

      // Right side

      auto layout_right = new QFormLayout (this);
      layout->addLayout(layout_right);

      auto settings_tabs = new QTabWidget (this);
      layout_right->addWidget(settings_tabs);

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

      render_settings_box_layout->addRow(new QLabel("Resolution:"));
      auto resolution = new QComboBox(this);
      resolution->addItem("256");
      resolution->addItem("512");
      resolution->addItem("1024");
      resolution->addItem("2048");
      resolution->addItem("4096");
      resolution->setCurrentText("512");

      render_settings_box_layout->addRow (resolution);

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

      _progress_bar = new QProgressBar(this);
      _progress_bar->setRange(0, 4096);
      generate_layout->addRow (_progress_bar);

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

      connect ( resolution, &QComboBox::currentTextChanged
          , [&] (QString s)
                {
                  _render_settings.resolution = s.toInt();
                }
      );

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

      // Selection

      QObject::connect
          ( _minimap_widget,  &minimap_widget::tile_clicked
              , [this, world] (QPoint tile)
            {
              if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
              {
                int x = tile.x() - 1;
                int y = tile.y() - 1;

                for (int i = 0; i < 3; ++i)
                {
                  for (int j = 0; j < 3; ++j)
                  {
                    if (world->mapIndex.hasTile(tile_index(x + i, y + j)))
                    {
                      _render_settings.selected_tiles[64 * (x + i) + (y + j)]
                        = !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
                    }

                  }
                }
              }
              else
              {
                if (world->mapIndex.hasTile(tile_index(tile.x(), tile.y())))
                {
                  _render_settings.selected_tiles[64 * tile.x() + tile.y()]
                    = !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
                }
              }

              update();
            }
          );

      QObject::connect
          ( _minimap_widget,  &minimap_widget::reset_selection
              , [this, world] ()
            {
              _render_settings.selected_tiles.fill(false);
            }
          );

    }

    void MinimapCreator::changeRadius(float change)
    {
      _radius_spin->setValue (_radius + change);
    }

    QSize MinimapCreator::sizeHint() const
    {
      return QSize(width(), height());
    }

    void MinimapCreator::wheelEvent(QWheelEvent* event)
    {

      if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
      {
        const int degrees = event->angleDelta().y() / 8;
        int steps = degrees / 15;

        auto base_size = _minimap_widget->width();

        if (steps > 0)
        {
          auto new_size = std::max(512, base_size + 64);
          _minimap_widget->setFixedSize(new_size, new_size);
        }
        else
        {
          auto new_size = std::min(4096, base_size - 64);
          _minimap_widget->setFixedSize(new_size, new_size);
        }

        event->ignore();
      }

    }
  }
}
