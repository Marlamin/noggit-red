// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QLabel>
#include <QWidget>
#include <QSettings>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>

#include <boost/optional.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

#include <noggit/ui/minimap_widget.hpp>

class MapView;
class World;

enum MinimapGenMode
{
  CURRENT_ADT,
  SELECTED_ADTS,
  MAP
};

struct MinimapRenderSettings
{
  MinimapGenMode export_mode; // Export mode

  // Render settings
  int resolution = 512;
  bool draw_m2 = false;
  bool draw_wmo = true;
  bool draw_water = true;
  bool draw_adt_grid = false;
  bool draw_elevation = false;

  // Filtering
  std::unordered_map<std::string, float> m2_model_filter_include; // filename, size category
  std::vector<uint32_t> m2_instance_filter_include; // include specific M2 instances
  std::vector<std::string> wmo_model_filter_exclude; // exclude WMOs by filename
  std::vector<uint32_t> wmo_instance_filter_exclude; // exclude specific WMO instances
};


namespace noggit
{

  namespace ui
  {
    class MinimapCreator : public QWidget
    {
    public:
      MinimapCreator (MapView* mapView, World* world, QWidget* parent = nullptr);

      void changeRadius(float change);

      float brushRadius() const { return _radius; }

      std::array<bool, 4096>* getSelectedTiles() { return &_selected_tiles; };

      MinimapRenderSettings* getMinimapRenderSettings() { return &_render_settings; };

      QSize sizeHint() const override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      float _radius = 0.01f;
      MinimapRenderSettings _render_settings;
      QSlider* _radius_slider;
      QDoubleSpinBox* _radius_spin;
      minimap_widget* _minimap_widget;

      std::array<bool, 4096> _selected_tiles = {false};

    };
  }
}
