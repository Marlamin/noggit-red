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
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <glm/vec4.hpp>

#include <noggit/MinimapRenderSettings.hpp>
#include <noggit/ui/minimap_widget.hpp>


class MapView;
class World;



namespace Noggit
{

  namespace Ui
  {

    class MinimapCreator : public QWidget
    {
        Q_OBJECT
    public:
      MinimapCreator(MapView* mapView, World* world, QWidget* parent = nullptr);

      void changeRadius(float change);

      float brushRadius() const { return _radius; }

      //std::array<bool, 4096>* getSelectedTiles() { return &_render_settings.selected_tiles; };
      std::vector<char>* getSelectedTiles() { return &_render_settings.selected_tiles; };

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
