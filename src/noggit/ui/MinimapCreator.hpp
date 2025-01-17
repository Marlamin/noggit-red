// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once
#include <noggit/MinimapRenderSettings.hpp>

#include <QWidget>

#include <string>
#include <vector>

class MapView;
class World;

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QSlider;

namespace Noggit
{
  namespace Ui
  {
    class minimap_widget;

    class MinimapCreator : public QWidget
    {
        Q_OBJECT
    public:
      MinimapCreator(MapView* mapView, World* world, QWidget* parent = nullptr);

      void changeRadius(float change);

      float brushRadius() const;

      //std::array<bool, 4096>* getSelectedTiles() { return &_render_settings.selected_tiles; };
      std::vector<char>* getSelectedTiles();;

      MinimapRenderSettings* getMinimapRenderSettings();;

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

    signals:
      void onSave();

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

      QString getFileName() const;;
      void setFileName(const std::string& filename);;
      void setSizeCategory(float size_cat);;
      float getSizeCategory() const;;

    private:
      QLineEdit* _filename;
      QDoubleSpinBox* _size_category_spin;
    };

    class MinimapWMOModelFilterEntry : public QWidget
    {
    public:
      MinimapWMOModelFilterEntry(QWidget* parent = nullptr);

      QString getFileName() const;;
      void setFileName(const std::string& filename);;

    private:
      QLineEdit* _filename;
    };

    class MinimapInstanceFilterEntry : public QWidget
    {
    public:
      MinimapInstanceFilterEntry(QWidget* parent = nullptr);

      uint32_t getUid() const;;
      void setUid(uint32_t uid);;

    private:
      uint32_t _uid;
      QLabel* _uid_label;
    };
  }
}
