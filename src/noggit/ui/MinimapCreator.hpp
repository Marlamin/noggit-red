// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QApplication>
#include <qt-color-widgets/color_selector.hpp>

#include <boost/optional.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <math/vector_4d.hpp>
#include <math/vector_3d.hpp>

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
  MinimapGenMode export_mode;
  std::string file_format = ".blp";

  // Render settings
  int resolution = 512;
  bool draw_m2 = false;
  bool draw_wmo = true;
  bool draw_water = true;
  bool draw_adt_grid = false;
  bool draw_elevation = false;
  bool draw_shadows = false;
  bool use_filters = false;
  bool combined_minimap = false;

  // Selection
  std::array<bool, 4096> selected_tiles = {false};

  // Filtering
  QListWidget* m2_model_filter_include;
  QListWidget* m2_instance_filter_include;
  QListWidget* wmo_model_filter_exclude;
  QListWidget* wmo_instance_filter_exclude;

  // Lighting
  math::vector_3d diffuse_color = {1.0, 0.532352924, 0.0};
  math::vector_3d ambient_color = {0.407770514, 0.508424163, 0.602650642};
  math::vector_4d ocean_color_light = {0.0693173409, 0.294008732, 0.348329663, 0.75};
  math::vector_4d ocean_color_dark = {0.000762581825, 0.113907099, 0.161220074, 1.0};
  math::vector_4d river_color_light = {0.308351517, 0.363725543, 0.0798838138, 0.5};
  math::vector_4d river_color_dark = {0.19945538, 0.320697188, 0.332425594, 1.0};

};


namespace noggit
{

  namespace ui
  {

    class MinimapCreator : public QWidget
    {
    public:
      MinimapCreator(MapView* mapView, World* world, QWidget* parent = nullptr);

      void changeRadius(float change);

      float brushRadius() const { return _radius; }

      std::array<bool, 4096>* getSelectedTiles() { return &_render_settings.selected_tiles; };

      MinimapRenderSettings* getMinimapRenderSettings() { return &_render_settings; };

      QSize sizeHint() const override;

      void includeM2Model(std::string filename, float size_cat = 0.0f);
      void unincludeM2Model(std::string filename);
      void includeM2Instance(uint32_t uid);
      void unincludeM2Instance(uint32_t uid);

      void excludeWMOModel(std::string filename);
      void unexcludeWMOModel(std::string filename);
      void excludeWMOInstance(uint32_t uid);
      void unexcludeWMOInstance(uint32_t uid);

      void loadFiltersFromJSON();
      void saveFiltersToJSON();

    private:
      float _radius = 0.01f;
      MinimapRenderSettings _render_settings;
      QSlider* _radius_slider;
      QDoubleSpinBox* _radius_spin;
      minimap_widget* _minimap_widget;
      QListWidget* _m2_model_filter_include;
      QListWidget* _m2_instance_filter_include;
      QListWidget* _wmo_model_filter_exclude;
      QListWidget* _wmo_instance_filter_exclude;

    };

    class MinimapM2ModelFilterEntry : public QWidget
    {
    public:
      MinimapM2ModelFilterEntry(QWidget* parent = nullptr);

      QString getFileName() { return _filename->text(); };
      void setFileName(const std::string& filename) { _filename->setText(QString(filename.c_str())); };
      void setSizeCategory(float size_cat) {_size_category_spin->setValue(size_cat); };
      float getSizeCategory() { return static_cast<float>(_size_category_spin->value()); };

    private:
      QLineEdit* _filename;
      QDoubleSpinBox* _size_category_spin;
    };

    class MinimapWMOModelFilterEntry : public QWidget
    {
    public:
      MinimapWMOModelFilterEntry(QWidget* parent = nullptr);

      QString getFileName() { return _filename->text(); };
      void setFileName(const std::string& filename) { _filename->setText(QString(filename.c_str())); };

    private:
      QLineEdit* _filename;
    };

    class MinimapInstanceFilterEntry : public QWidget
    {
    public:
      MinimapInstanceFilterEntry(QWidget* parent = nullptr);

      uint32_t getUid() { return _uid; };
      void setUid(uint32_t uid) { _uid = uid; _uid_label->setText(QString::fromStdString(std::to_string(uid))); };

    private:
      uint32_t _uid;
      QLabel* _uid_label;
    };
  }
}
