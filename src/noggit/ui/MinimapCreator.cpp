// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "MinimapCreator.hpp"
#include "font_awesome.hpp"

#include <noggit/MapView.h>
#include <noggit/World.h>
#include <noggit/Log.h>

#include <util/qt/overload.hpp>

#include <QFormLayout>
#include <QGridLayout>
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
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QByteArray>
#include <QPalette>

namespace noggit
{
  namespace ui
  {
    MinimapCreator::MinimapCreator (
        MapView* mapView,
        World* world,
        QWidget* parent ) : QWidget(parent)
    {
      //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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
      settings_tabs->setFixedWidth(300);
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

      auto file_format = new QComboBox(this);
      file_format->addItem(".blp");
      file_format->addItem(".png");
      file_format->setCurrentText(".blp");

      render_settings_box_layout->addRow (file_format);

      auto draw_elements_box_layout = new QGridLayout();
      render_settings_box_layout->addItem(draw_elements_box_layout);

      auto draw_models = new QCheckBox("Models", render_settings_box);
      draw_models->setChecked(_render_settings.draw_m2);
      draw_elements_box_layout->addWidget(draw_models, 0, 0);

      auto draw_wmos = new QCheckBox("WMOs", render_settings_box);
      draw_wmos->setChecked(_render_settings.draw_wmo);
      draw_elements_box_layout->addWidget(draw_wmos, 1, 0);

      auto draw_water = new QCheckBox("Water", render_settings_box);
      draw_water->setChecked(_render_settings.draw_water);
      draw_elements_box_layout->addWidget(draw_water, 2, 0);

      auto use_filters = new QCheckBox("Filter", render_settings_box);
      use_filters->setChecked(_render_settings.use_filters);
      draw_elements_box_layout->addWidget(use_filters, 3, 0);

      auto draw_adt = new QCheckBox("ADT grid", render_settings_box);
      draw_adt->setChecked(_render_settings.draw_adt_grid);
      draw_elements_box_layout->addWidget(draw_adt, 0, 1);

      auto draw_elevation = new QCheckBox("Elevation", render_settings_box);
      draw_elevation->setChecked(_render_settings.draw_elevation);
      draw_elements_box_layout->addWidget(draw_elevation, 1, 1);

      auto draw_shadows = new QCheckBox("Shadows", render_settings_box);
      draw_elevation->setChecked(_render_settings.draw_shadows);
      draw_elements_box_layout->addWidget(draw_shadows, 2, 1);

      auto combined_minimap = new QCheckBox("Combine", render_settings_box);
      combined_minimap->setChecked(_render_settings.combined_minimap);
      draw_elements_box_layout->addWidget(combined_minimap, 3, 1);


      _progress_bar = new QProgressBar(this);
      _progress_bar->setRange(0, 4096);
      generate_layout->addRow(_progress_bar);

      // Filter
      auto filter_widget = new QWidget(this);
      filter_widget->setContentsMargins(0, 0, 0, 0);
      auto filter_layout = new QFormLayout(filter_widget);
      filter_layout->setContentsMargins(0, 0, 0, 0);

      settings_tabs->addTab(filter_widget, "Filter");

      auto filter_tabs = new QTabWidget(filter_widget);
      filter_tabs->setTabPosition(QTabWidget::East);
      filter_tabs->setDocumentMode(true);
      filter_layout->addRow(filter_tabs);

      // M2 model include tab
      auto m2_include_widget = new QWidget(filter_tabs);
      filter_tabs->addTab(m2_include_widget, "M2 Model");

      auto m2_include_box_layout = new QFormLayout(m2_include_widget);
      m2_include_box_layout->setContentsMargins(0, 0, 0, 0);
      m2_include_widget->setLayout(m2_include_box_layout);

      auto search_filter_m2 = new QLineEdit(this);
      search_filter_m2->setMinimumWidth(220);
      m2_include_box_layout->addRow("Filter:", search_filter_m2);

      _m2_model_filter_include = new QListWidget(m2_include_widget);
      _render_settings.m2_model_filter_include = _m2_model_filter_include;
      _m2_model_filter_include->setAlternatingRowColors(true);

      m2_include_box_layout->addRow(_m2_model_filter_include);

      auto m2_include_box_layout_btns = new QHBoxLayout(m2_include_widget);
      m2_include_box_layout->addRow(m2_include_box_layout_btns);

      auto add_btn = new QPushButton("Add", m2_include_widget);
      add_btn->setIcon(font_awesome_icon(font_awesome::plus));
      m2_include_box_layout_btns->addWidget(add_btn);

      auto remove_btn = new QPushButton("Remove", m2_include_widget);
      remove_btn->setIcon(font_awesome_icon(font_awesome::timescircle));
      m2_include_box_layout_btns->addWidget(remove_btn);

      auto clear_btn = new QPushButton("Clear all", m2_include_widget);
      clear_btn->setIcon(font_awesome_icon(font_awesome::trash));
      m2_include_box_layout_btns->addWidget(clear_btn);

      // M2 instance include tab
      auto m2_instance_include_widget = new QWidget(filter_tabs);
      filter_tabs->addTab(m2_instance_include_widget, "M2 Instance");

      auto m2_instance_include_box_layout = new QFormLayout(m2_instance_include_widget);
      m2_instance_include_box_layout->setContentsMargins(0, 0, 0, 0);
      m2_instance_include_widget->setLayout(m2_instance_include_box_layout);

      auto search_filter_m2i = new QLineEdit(this);
      search_filter_m2i->setMinimumWidth(220);
      m2_instance_include_box_layout->addRow("Filter:", search_filter_m2i);

      _m2_instance_filter_include = new QListWidget(m2_instance_include_widget);
      _render_settings.m2_instance_filter_include = _m2_instance_filter_include;
      _m2_instance_filter_include->setAlternatingRowColors(true);

      m2_instance_include_box_layout->addRow(_m2_instance_filter_include);

      auto m2_instance_include_box_layout_btns = new QHBoxLayout(m2_instance_include_widget);
      m2_instance_include_box_layout->addRow(m2_instance_include_box_layout_btns);

      auto add_btn_m2i = new QPushButton("Add", m2_instance_include_widget);
      add_btn_m2i->setIcon(font_awesome_icon(font_awesome::plus));
      m2_instance_include_box_layout_btns->addWidget(add_btn_m2i);

      auto remove_btn_m2i = new QPushButton("Remove", m2_instance_include_widget);
      remove_btn_m2i->setIcon(font_awesome_icon(font_awesome::timescircle));
      m2_instance_include_box_layout_btns->addWidget(remove_btn_m2i);

      auto clear_btn_m2i = new QPushButton("Clear all", m2_instance_include_widget);
      clear_btn_m2i->setIcon(font_awesome_icon(font_awesome::trash));
      m2_instance_include_box_layout_btns->addWidget(clear_btn_m2i);

      // WMO model exclude tab
      auto wmo_exclude_widget = new QWidget(filter_tabs);
      filter_tabs->addTab(wmo_exclude_widget, "WMO Model");

      auto wmo_exclude_box_layout = new QFormLayout(wmo_exclude_widget);
      wmo_exclude_box_layout->setContentsMargins(0, 0, 0, 0);
      wmo_exclude_widget->setLayout(wmo_exclude_box_layout);

      auto search_filter_wmo = new QLineEdit(this);
      search_filter_wmo->setMinimumWidth(220);
      wmo_exclude_box_layout->addRow("Filter:", search_filter_wmo);

      _wmo_model_filter_exclude = new QListWidget(wmo_exclude_widget);
      _render_settings.wmo_model_filter_exclude = _wmo_model_filter_exclude;
      _wmo_model_filter_exclude->setAlternatingRowColors(true);

      wmo_exclude_box_layout->addRow(_wmo_model_filter_exclude);

      auto wmo_exclude_box_layout_btns = new QHBoxLayout(wmo_exclude_widget);
      wmo_exclude_box_layout->addRow(wmo_exclude_box_layout_btns);

      auto add_btn_wmo = new QPushButton("Add", wmo_exclude_widget);
      add_btn_wmo->setIcon(font_awesome_icon(font_awesome::plus));
      wmo_exclude_box_layout_btns->addWidget(add_btn_wmo);

      auto remove_btn_wmo = new QPushButton("Remove", wmo_exclude_widget);
      remove_btn_wmo->setIcon(font_awesome_icon(font_awesome::timescircle));
      wmo_exclude_box_layout_btns->addWidget(remove_btn_wmo);

      auto clear_btn_wmo = new QPushButton("Clear all", wmo_exclude_widget);
      clear_btn_wmo->setIcon(font_awesome_icon(font_awesome::trash));
      wmo_exclude_box_layout_btns->addWidget(clear_btn_wmo);

      // WMO instance exclude tab
      auto wmo_instance_exclude_widget = new QWidget(filter_tabs);
      filter_tabs->addTab(wmo_instance_exclude_widget, "WMO Instance");

      auto wmo_instance_exclude_box_layout = new QFormLayout(wmo_instance_exclude_widget);
      wmo_instance_exclude_box_layout->setContentsMargins(0, 0, 0, 0);
      wmo_instance_exclude_widget->setLayout(wmo_instance_exclude_box_layout);

      auto search_filter_wmo_i = new QLineEdit(this);
      search_filter_wmo_i->setMinimumWidth(220);
      wmo_instance_exclude_box_layout->addRow("Filter:", search_filter_wmo_i);

      _wmo_instance_filter_exclude = new QListWidget(wmo_instance_exclude_widget);
      _render_settings.wmo_instance_filter_exclude = _wmo_instance_filter_exclude;
      _wmo_instance_filter_exclude->setAlternatingRowColors(true);

      wmo_instance_exclude_box_layout->addRow(_wmo_instance_filter_exclude);

      auto wmo_instance_exclude_box_layout_btns = new QHBoxLayout(wmo_instance_exclude_widget);
      wmo_instance_exclude_box_layout->addRow(wmo_instance_exclude_box_layout_btns);

      auto add_btn_wmo_i = new QPushButton("Add", wmo_instance_exclude_widget);
      add_btn_wmo_i->setIcon(font_awesome_icon(font_awesome::plus));
      wmo_instance_exclude_box_layout_btns->addWidget(add_btn_wmo_i);

      auto remove_btn_wmo_i = new QPushButton("Remove", wmo_instance_exclude_widget);
      remove_btn_wmo_i->setIcon(font_awesome_icon(font_awesome::timescircle));
      wmo_instance_exclude_box_layout_btns->addWidget(remove_btn_wmo_i);

      auto clear_btn_wmo_i = new QPushButton("Clear all", wmo_instance_exclude_widget);
      clear_btn_wmo_i->setIcon(font_awesome_icon(font_awesome::trash));
      wmo_instance_exclude_box_layout_btns->addWidget(clear_btn_wmo_i);

      // Lighting

      auto lighting_widget = new QWidget(this);
      auto lighting_layout = new QFormLayout(filter_widget);
      lighting_widget->setLayout(lighting_layout);

      settings_tabs->addTab(lighting_widget, "Lighting");

      auto diffuse_color = new color_widgets::ColorSelector (this);
      diffuse_color->setDisplayMode (color_widgets::ColorSelector::NoAlpha);
      diffuse_color->setColor (QColor::fromRgbF (_render_settings.diffuse_color.x,
                                                 _render_settings.diffuse_color.y,
                                                 _render_settings.diffuse_color.z));
      diffuse_color->setMinimumHeight(25);
      diffuse_color->setMinimumWidth(100);


      lighting_layout->addRow("Diffuse color:", diffuse_color);

      auto ambient_color = new color_widgets::ColorSelector (this);
      ambient_color->setDisplayMode (color_widgets::ColorSelector::NoAlpha);
      ambient_color->setColor (QColor::fromRgbF (_render_settings.ambient_color.x,
                                                 _render_settings.ambient_color.y,
                                                 _render_settings.ambient_color.z));
      ambient_color->setMinimumHeight(25);
      ambient_color->setMinimumWidth(100);

      lighting_layout->addRow("Ambient color:", ambient_color);

      auto ocean_color_light = new color_widgets::ColorSelector (this);
      ocean_color_light->setColor (QColor::fromRgbF (_render_settings.ocean_color_light.x,
                                                 _render_settings.ocean_color_light.y,
                                                 _render_settings.ocean_color_light.z,
                                                 _render_settings.ocean_color_light.w));
      ocean_color_light->setMinimumHeight(25);
      ocean_color_light->setMinimumWidth(100);

      lighting_layout->addRow("Ocean light color:", ocean_color_light);

      auto ocean_color_dark = new color_widgets::ColorSelector (this);
      ocean_color_dark->setColor (QColor::fromRgbF (_render_settings.ocean_color_dark.x,
                                                 _render_settings.ocean_color_dark.y,
                                                 _render_settings.ocean_color_dark.z,
                                                 _render_settings.ocean_color_dark.w));
      ocean_color_dark->setMinimumHeight(25);
      ocean_color_dark->setMinimumWidth(100);

      lighting_layout->addRow("Ocean dark color:", ocean_color_dark);

      auto river_color_light = new color_widgets::ColorSelector (this);
      river_color_light->setColor (QColor::fromRgbF (_render_settings.river_color_light.x,
                                                 _render_settings.river_color_light.y,
                                                 _render_settings.river_color_light.z,
                                                 _render_settings.river_color_light.w));
      river_color_light->setMinimumHeight(25);
      river_color_light->setMinimumWidth(100);

      lighting_layout->addRow("River light color:", river_color_light);

      auto river_color_dark = new color_widgets::ColorSelector (this);
      river_color_dark->setColor (QColor::fromRgbF (_render_settings.river_color_dark.x,
                                                 _render_settings.river_color_dark.y,
                                                 _render_settings.river_color_dark.z,
                                                 _render_settings.river_color_dark.w));
      river_color_dark->setMinimumHeight(25);
      river_color_dark->setMinimumWidth(100);

      lighting_layout->addRow("River dark color:", river_color_dark);

      loadFiltersFromJSON();

      // Connections

      // Filter buttons

      // Lighting colors
      connect(diffuse_color, &color_widgets::ColorSelector::colorChanged,
          [this] (QColor new_color)
          {

            _render_settings.diffuse_color = {static_cast<float>(new_color.redF()),
                                              static_cast<float>(new_color.greenF()),
                                              static_cast<float>(new_color.blueF())};
                                             }
      );


      connect(ambient_color, &color_widgets::ColorSelector::colorChanged,
              [this] (QColor new_color)
              {

                _render_settings.ambient_color = {static_cast<float>(new_color.redF()),
                                                  static_cast<float>(new_color.greenF()),
                                                  static_cast<float>(new_color.blueF())};
              }
      );

      connect(ocean_color_light, &color_widgets::ColorSelector::colorChanged,
              [this] (QColor new_color)
              {

                _render_settings.ocean_color_light = {static_cast<float>(new_color.redF()),
                                                      static_cast<float>(new_color.greenF()),
                                                      static_cast<float>(new_color.blueF()),
                                                      static_cast<float>(new_color.alphaF())};
              }
      );

      connect(ocean_color_dark, &color_widgets::ColorSelector::colorChanged,
              [this] (QColor new_color)
              {

                _render_settings.ocean_color_dark = {static_cast<float>(new_color.redF()),
                                                    static_cast<float>(new_color.greenF()),
                                                    static_cast<float>(new_color.blueF()),
                                                    static_cast<float>(new_color.alphaF())};
              }
      );

      connect(river_color_light, &color_widgets::ColorSelector::colorChanged,
              [this] (QColor new_color)
              {

                _render_settings.river_color_light = {static_cast<float>(new_color.redF()),
                                                      static_cast<float>(new_color.greenF()),
                                                      static_cast<float>(new_color.blueF()),
                                                      static_cast<float>(new_color.alphaF())};
              }
      );

      connect(river_color_dark , &color_widgets::ColorSelector::colorChanged,
              [this] (QColor new_color)
              {

                _render_settings.river_color_dark = {static_cast<float>(new_color.redF()),
                                                    static_cast<float>(new_color.greenF()),
                                                    static_cast<float>(new_color.blueF()),
                                                    static_cast<float>(new_color.alphaF())};
              }
      );

      // M2 model filter
      connect(add_btn, &QPushButton::clicked,
          [=]()
                {
                  if (!world->has_selection())
                  {
                    return;
                  }

                  for (auto& selection : world->current_selection())
                  {
                    if (selection.which() == eEntry_Model)
                    {
                      includeM2Model(boost::get<selected_model_type>(selection)->model->filename);
                    }

                  }

                }
      );

      connect(remove_btn, &QPushButton::clicked,
              [=]()
              {

                if (!world->get_selected_model_count())
                {
                  for (auto item : _m2_model_filter_include->selectedItems())
                  {
                    auto item_ = _m2_model_filter_include->takeItem(_m2_model_filter_include->row(item));
                    delete item_;
                  }

                  return;
                }

                for (auto& selection : world->current_selection())
                {
                  if (selection.which() == eEntry_Model)
                  {
                    unincludeM2Model(boost::get<selected_model_type>(selection)->model->filename);
                  }

                }

              }
      );


      connect(clear_btn, &QPushButton::clicked,
              [=]()
              {
                _m2_model_filter_include->clear();
              }
      );

      // M2 instance filter
      connect(add_btn_m2i, &QPushButton::clicked,
          [=]()
                {
                  if (!world->has_selection())
                  {
                    return;
                  }

                  for (auto& selection : world->current_selection())
                  {
                    if (selection.which() == eEntry_Model)
                    {
                      includeM2Instance(boost::get<selected_model_type>(selection)->uid);
                    }

                  }

                }
      );

      connect(remove_btn_m2i, &QPushButton::clicked,
              [=]()
              {

                if (!world->get_selected_model_count())
                {
                  for (auto item : _m2_instance_filter_include->selectedItems())
                  {
                    auto item_ = _m2_instance_filter_include->takeItem(_m2_instance_filter_include->row(item));
                    delete item_;
                  }

                  return;
                }

                for (auto& selection : world->current_selection())
                {

                  if (selection.which() == eEntry_Model)
                  {
                    unincludeM2Instance(boost::get<selected_model_type>(selection)->uid);
                  }

                }

              }
      );

      connect(clear_btn_m2i, &QPushButton::clicked,
              [=]()
              {
                _m2_instance_filter_include->clear();
              }
      );

      // WMO model filter
      connect(add_btn_wmo, &QPushButton::clicked,
              [=]()
              {
                if (!world->has_selection())
                {
                  return;
                }

                for (auto& selection : world->current_selection())
                {
                  if (selection.which() == eEntry_WMO)
                  {
                    excludeWMOModel(boost::get<selected_wmo_type>(selection)->wmo->filename);
                  }

                }

              }
      );

      connect(remove_btn_wmo, &QPushButton::clicked,
              [=]()
              {

                if (!world->get_selected_model_count())
                {
                  for (auto item : _wmo_model_filter_exclude->selectedItems())
                  {
                    auto item_ = _wmo_model_filter_exclude->takeItem(_wmo_model_filter_exclude->row(item));
                    delete item_;
                  }

                  return;
                }

                for (auto& selection : world->current_selection())
                {

                  if (selection.which() == eEntry_WMO)
                  {
                    unexcludeWMOModel(boost::get<selected_wmo_type>(selection)->wmo->filename);
                  }

                }

              }
      );

      connect(clear_btn_wmo, &QPushButton::clicked,
              [=]()
              {
                _wmo_model_filter_exclude->clear();
              }
      );

      // WMO instance filter
      connect(add_btn_wmo_i, &QPushButton::clicked,
              [=]()
              {
                if (!world->has_selection())
                {
                  return;
                }

                for (auto& selection : world->current_selection())
                {
                  if (selection.which() == eEntry_WMO)
                  {
                    excludeWMOInstance(boost::get<selected_wmo_type>(selection)->mUniqueID);
                  }

                }

              }
      );

      connect(remove_btn_wmo_i, &QPushButton::clicked,
              [=]()
              {

                if (!world->get_selected_model_count())
                {
                  for (auto item : _wmo_instance_filter_exclude->selectedItems())
                  {
                    auto item_ = _wmo_instance_filter_exclude->takeItem(_wmo_instance_filter_exclude->row(item));
                    delete item_;
                  }

                  return;
                }

                for (auto& selection : world->current_selection())
                {

                  if (selection.which() == eEntry_WMO)
                  {
                    unexcludeWMOInstance(boost::get<selected_wmo_type>(selection)->mUniqueID);
                  }

                }

              }
      );

      connect(clear_btn_wmo_i, &QPushButton::clicked,
              [=]()
              {
                _wmo_instance_filter_exclude->clear();
              }
      );

      // Search filters

      connect ( search_filter_m2, qOverload<const QString&> (&QLineEdit::textChanged)
          , [&] (const QString& s)
                {
                  for (int i = 0; i < _m2_model_filter_include->count(); ++i)
                  {
                    MinimapM2ModelFilterEntry* item_wgt = reinterpret_cast<MinimapM2ModelFilterEntry*>(
                        _m2_model_filter_include->itemWidget(_m2_model_filter_include->item(i)));

                    std::string filename = item_wgt->getFileName().toStdString();
                    std::string filter = s.toStdString();
                    bool isHidden = !filter.empty() && filename.find(filter) == std::string::npos;
                    _m2_model_filter_include->item(i)->setHidden(isHidden);
                  }
                }
      );

      connect ( search_filter_m2i, qOverload<const QString&> (&QLineEdit::textChanged)
          , [&] (const QString& s)
                {
                  for (int i = 0; i < _m2_instance_filter_include->count(); ++i)
                  {
                    MinimapInstanceFilterEntry* item_wgt = reinterpret_cast<MinimapInstanceFilterEntry*>(
                        _m2_instance_filter_include->itemWidget(_m2_instance_filter_include->item(i)));

                    std::string uid = std::to_string(item_wgt->getUid());
                    std::string filter = s.toStdString();
                    bool isHidden = !filter.empty() && uid.find(filter) == std::string::npos;
                    _m2_instance_filter_include->item(i)->setHidden(isHidden);
                  }
                }
      );

      connect ( search_filter_wmo, qOverload<const QString&> (&QLineEdit::textChanged)
          , [&] (const QString& s)
                {
                  for (int i = 0; i < _wmo_model_filter_exclude->count(); ++i)
                  {
                    MinimapWMOModelFilterEntry* item_wgt = reinterpret_cast<MinimapWMOModelFilterEntry*>(
                        _wmo_model_filter_exclude->itemWidget(_wmo_model_filter_exclude->item(i)));

                    std::string filename = item_wgt->getFileName().toStdString();
                    std::string filter = s.toStdString();
                    bool isHidden = !filter.empty() && filename.find(filter) == std::string::npos;
                    _wmo_model_filter_exclude->item(i)->setHidden(isHidden);
                  }
                }
      );

      connect ( search_filter_wmo_i, qOverload<const QString&> (&QLineEdit::textChanged)
          , [&] (const QString& s)
                {
                  for (int i = 0; i < _wmo_instance_filter_exclude->count(); ++i)
                  {
                    MinimapInstanceFilterEntry* item_wgt = reinterpret_cast<MinimapInstanceFilterEntry*>(
                        _wmo_instance_filter_exclude->itemWidget(_wmo_instance_filter_exclude->item(i)));

                    std::string uid = std::to_string(item_wgt->getUid());
                    std::string filter = s.toStdString();
                    bool isHidden = !filter.empty() && uid.find(filter) == std::string::npos;
                    _wmo_instance_filter_exclude->item(i)->setHidden(isHidden);
                  }
                }
      );

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

      connect ( file_format, &QComboBox::currentTextChanged
          , [&] (QString s)
                {
                  _render_settings.file_format = s.toStdString();
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

      connect (use_filters, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.use_filters = s;
      });

      connect (draw_shadows, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.draw_shadows = s;
      });

      connect (combined_minimap, &QCheckBox::stateChanged, [this] (int s)
      {
        _render_settings.combined_minimap = s;
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

    void MinimapCreator::includeM2Model(std::string filename, float size_cat)
    {
      bool already_added = false;

      for (int i = 0; i < _m2_model_filter_include->count(); ++i)
      {
        if (!reinterpret_cast<MinimapM2ModelFilterEntry*>(_m2_model_filter_include->itemWidget(
            _m2_model_filter_include->item(i)))->getFileName().toStdString().compare(filename))
        {
          already_added = true;
          break;
        }
      }

      if (already_added)
      {
        return;
      }

      auto item = new QListWidgetItem();
      _m2_model_filter_include->addItem(item);
      auto entry_wgt = new MinimapM2ModelFilterEntry(this);
      entry_wgt->setFileName(filename);
      entry_wgt->setSizeCategory(size_cat);
      item->setSizeHint(entry_wgt->minimumSizeHint());
      _m2_model_filter_include->setItemWidget(item, entry_wgt);
    }

    void MinimapCreator::unincludeM2Model(std::string filename)
    {

      for (int i = 0; i < _m2_model_filter_include->count(); ++i )
      {
        if (!reinterpret_cast<MinimapM2ModelFilterEntry*>(_m2_model_filter_include->itemWidget(
            _m2_model_filter_include->item(i)))->getFileName().toStdString().compare(filename))
        {
          auto item = _m2_model_filter_include->takeItem(i);
          delete item;
        }
      }

    }

    void MinimapCreator::includeM2Instance(uint32_t uid)
    {
      bool already_added = false;

      for (int i = 0; i < _m2_instance_filter_include->count(); ++i)
      {
        if (reinterpret_cast<MinimapInstanceFilterEntry*>(_m2_instance_filter_include->itemWidget(
            _m2_instance_filter_include->item(i)))->getUid() == uid)
        {
          already_added = true;
          break;
        }
      }

      if (already_added)
      {
        return;
      }

      auto item = new QListWidgetItem();
      _m2_instance_filter_include->addItem(item);
      auto entry_wgt = new MinimapInstanceFilterEntry(this);

      QPalette pal = entry_wgt->palette();
      pal.setColor(QPalette::Window, Qt::black);
      //entry_wgt->setAutoFillBackground(true);
      //entry_wgt->setPalette(pal);

      entry_wgt->setUid(uid);
      item->setSizeHint(entry_wgt->minimumSizeHint());
      _m2_instance_filter_include->setItemWidget(item, entry_wgt);
    }

    void MinimapCreator::unincludeM2Instance(uint32_t uid)
    {
      for (int i = 0; i < _m2_model_filter_include->count(); ++i )
      {
        if (reinterpret_cast<MinimapInstanceFilterEntry*>(_m2_instance_filter_include->itemWidget(
            _m2_instance_filter_include->item(i)))->getUid() == uid)
        {
          auto item = _m2_model_filter_include->takeItem(i);
          delete item;
        }
      }
    }

    void MinimapCreator::excludeWMOModel(std::string filename)
    {
      bool already_added = false;

      for (int i = 0; i < _wmo_model_filter_exclude->count(); ++i)
      {
        if (!reinterpret_cast<MinimapWMOModelFilterEntry*>(_wmo_model_filter_exclude->itemWidget(
            _wmo_model_filter_exclude->item(i)))->getFileName().toStdString().compare(filename))
        {
          already_added = true;
          break;
        }
      }

      if (already_added)
      {
        return;
      }

      auto item = new QListWidgetItem();
      _wmo_model_filter_exclude->addItem(item);
      auto entry_wgt = new MinimapWMOModelFilterEntry(this);
      entry_wgt->setFileName(filename);
      item->setSizeHint(entry_wgt->minimumSizeHint());
      _wmo_model_filter_exclude->setItemWidget(item, entry_wgt);
    }

    void MinimapCreator::unexcludeWMOModel(std::string filename)
    {

      for (int i = 0; i < _wmo_model_filter_exclude->count(); ++i )
      {
        if (!reinterpret_cast<MinimapWMOModelFilterEntry*>(_wmo_model_filter_exclude->itemWidget(
            _wmo_model_filter_exclude->item(i)))->getFileName().toStdString().compare(filename))
        {
          auto item = _wmo_model_filter_exclude->takeItem(i);
          delete item;
        }
      }

    }

    void MinimapCreator::excludeWMOInstance(uint32_t uid)
    {
      bool already_added = false;

      for (int i = 0; i < _wmo_instance_filter_exclude->count(); ++i)
      {
        if (reinterpret_cast<MinimapInstanceFilterEntry*>(_wmo_instance_filter_exclude->itemWidget(
            _wmo_instance_filter_exclude->item(i)))->getUid() == uid)
        {
          already_added = true;
          break;
        }
      }

      if (already_added)
      {
        return;
      }

      auto item = new QListWidgetItem();
      _wmo_instance_filter_exclude->addItem(item);
      auto entry_wgt = new MinimapInstanceFilterEntry(this);
      entry_wgt->setUid(uid);
      item->setSizeHint(entry_wgt->minimumSizeHint());
      _wmo_instance_filter_exclude->setItemWidget(item, entry_wgt);
    }

    void MinimapCreator::unexcludeWMOInstance(uint32_t uid)
    {
      for (int i = 0; i < _wmo_instance_filter_exclude->count(); ++i )
      {
        if (reinterpret_cast<MinimapInstanceFilterEntry*>(_wmo_instance_filter_exclude->itemWidget(
            _wmo_instance_filter_exclude->item(i)))->getUid() == uid)
        {
          auto item = _wmo_instance_filter_exclude->takeItem(i);
          delete item;
        }
      }
    }

    void MinimapCreator::loadFiltersFromJSON()
    {
      QSettings settings;
      QString str = settings.value ("project/path").toString();
      if (!(str.endsWith('\\') || str.endsWith('/')))
      {
        str += "/";
      }

      QFile json_file = QFile(str + "/noggit_mmap_filters.json");
      if (!json_file.open(QIODevice::ReadOnly))
      {
        return;
      }

      QByteArray save_data = json_file.readAll();
      QJsonDocument json_doc(QJsonDocument::fromJson(save_data));

      if (json_doc.isObject())
      {
        QJsonObject doc_object = json_doc.object();

        // M2 models
        QJsonArray  m2_models = doc_object["m2_models"].toArray();

        for (const auto& m2_model_val : m2_models)
        {
          if (m2_model_val.isObject())
          {
            QJsonObject m2_model = m2_model_val.toObject();

            QString filename = m2_model["filename"].toString();
            double size_cat = m2_model["size_category"].toDouble();

            includeM2Model(filename.toStdString(), static_cast<float>(size_cat));

          }
        }

        // M2 instances
        QJsonArray  m2_instances = doc_object["m2_instances"].toArray();

        for (const auto& m2_instance_val : m2_instances)
        {
          includeM2Instance(m2_instance_val.toInt());
        }

        // WMO models
        QJsonArray wmo_models = doc_object["wmo_models"].toArray();

        for (const auto& wmo_model_val : wmo_models)
        {
          excludeWMOModel(wmo_model_val.toString().toStdString());
        }

        // WMO instances
        QJsonArray wmo_instances = doc_object["wmo_instances"].toArray();

        for (const auto& wmo_instance_val : wmo_instances)
        {
          excludeWMOInstance(wmo_instance_val.toInt());
        }

      }

      json_file.close();

    }

    void MinimapCreator::saveFiltersToJSON()
    {
      QSettings settings;
      QString str = settings.value ("project/path").toString();
      if (!(str.endsWith('\\') || str.endsWith('/')))
      {
        str += "/";
      }

      QJsonDocument json_doc;

      QFile json_file = QFile(str + "/noggit_mmap_filters.json");
      if (!json_file.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate))
      {
        LogError << "Unable to save minimap creator model filters to JSON document." << std::endl;
        return;
      }

      json_doc = QJsonDocument();

      QJsonObject json_root_obj = QJsonObject();
      QJsonArray m2_models = QJsonArray();
      QJsonArray m2_instances = QJsonArray();
      QJsonArray wmo_models = QJsonArray();
      QJsonArray wmo_instances = QJsonArray();

      // m2 models
      for (int i = 0; i < _m2_model_filter_include->count(); ++i)
      {
        auto item_wgt = reinterpret_cast<MinimapM2ModelFilterEntry*>(_m2_model_filter_include->itemWidget(
            _m2_model_filter_include->item(i)));

        QJsonObject item = QJsonObject();
        item.insert("filename", QJsonValue(item_wgt->getFileName()));
        item.insert("size_category", QJsonValue(static_cast<double>(item_wgt->getSizeCategory())));
        m2_models.append(item);
      }

      // m2 instances
      for (int i = 0; i < _m2_instance_filter_include->count(); ++i)
      {
        auto item_wgt = reinterpret_cast<MinimapInstanceFilterEntry*>(_m2_instance_filter_include->itemWidget(
            _m2_instance_filter_include->item(i)));

        m2_instances.append(QJsonValue(static_cast<int>(item_wgt->getUid())));
      }

      // wmo models
      for (int i = 0; i < _wmo_model_filter_exclude->count(); ++i)
      {
        auto item_wgt = reinterpret_cast<MinimapWMOModelFilterEntry*>(_wmo_model_filter_exclude->itemWidget(
            _wmo_model_filter_exclude->item(i)));

        wmo_models.append(QJsonValue(item_wgt->getFileName()));
      }

      // wmo instances
      for (int i = 0; i < _wmo_instance_filter_exclude->count(); ++i)
      {
        auto item_wgt = reinterpret_cast<MinimapInstanceFilterEntry*>(_wmo_instance_filter_exclude->itemWidget(
            _wmo_instance_filter_exclude->item(i)));

        wmo_instances.append(QJsonValue(static_cast<int>(item_wgt->getUid())));
      }

      json_root_obj.insert("m2_models", m2_models);
      json_root_obj.insert("m2_instances", m2_instances);
      json_root_obj.insert("wmo_models", wmo_models);
      json_root_obj.insert("wmo_instances", wmo_instances);

      json_doc.setObject(json_root_obj);
      json_file.write(json_doc.toJson());
      json_file.close();

    }

    MinimapM2ModelFilterEntry::MinimapM2ModelFilterEntry(QWidget* parent) : QWidget(parent)
    {
      setAttribute(Qt::WA_TranslucentBackground);
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins(5, 2, 5, 2);
      layout->addWidget(_filename = new QLineEdit(this));
      _filename->setEnabled(false);
      layout->addWidget(_size_category_spin = new QDoubleSpinBox(this));
      _size_category_spin->setRange (0.0f, 1000.0f);
      _size_category_spin->setValue (0.0);

    }

     MinimapWMOModelFilterEntry::MinimapWMOModelFilterEntry(QWidget* parent) : QWidget(parent)
    {
      setAttribute(Qt::WA_TranslucentBackground);
      auto layout = new QHBoxLayout(this);
      layout->addWidget(_filename = new QLineEdit(this));
      _filename->setAttribute(Qt::WA_TranslucentBackground);
      layout->setContentsMargins(5, 2, 5, 2);
      _filename->setEnabled(false);
    }

    MinimapInstanceFilterEntry::MinimapInstanceFilterEntry(QWidget* parent) : QWidget(parent)
    {
      setAttribute(Qt::WA_TranslucentBackground);
      auto layout = new QHBoxLayout(this);
      layout->addWidget(_uid_label = new QLabel(this));
      _uid_label->setAttribute(Qt::WA_TranslucentBackground);
      layout->setContentsMargins(5, 2, 5, 2);
    }

  }
}
