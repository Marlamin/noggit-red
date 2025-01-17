// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/BoolToggleProperty.hpp>
#include <noggit/Brush.h>
#include <noggit/TextureManager.h>
#include <noggit/unsigned_int_property.hpp>

#include <QJsonObject>
#include <QtWidgets/QDial>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

class World;
class MapView;

class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;
class QPushButton;
class QSpinBox;
class QTabWidget;

inline constexpr const char* STRING_EMPTY_DISPLAY = "-NONE-";

namespace Noggit
{
  namespace Ui
  {
    class CheckBox;
    class current_texture;
    class GroundEffectsTool;
    class texture_swapper;

    namespace Tools
    {
      namespace UiCommon
      {
        class ExtendedSlider;
      }

      class ImageMaskSelector;
    }

    class OpacitySlider : public QSlider{
    public:
        OpacitySlider(Qt::Orientation orientation, QWidget * parent = nullptr);
    
    protected:
        void paintEvent(QPaintEvent * event) override;
    };

    enum class texturing_mode
    {
      paint,
      swap,
      anim,
      ground_effect
    };

    /// <summary>
    /// ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// </summary>
    /// 
    /// 
    /// 
    /// 

    class texturing_tool : public QWidget
    {
        Q_OBJECT
    public:
      texturing_tool ( const glm::vec3* camera_pos
                     , MapView* map_view
                     , BoolToggleProperty* show_quick_palette
                     , QWidget* parent = nullptr
                     );

      ~texturing_tool(); // { _ground_effect_tool->deleteLater(); }; // { delete _ground_effect_tool; };

      float brush_radius() const;
      float hardness() const;
      bool show_unpaintable_chunks() const;

      void set_brush_level (float level);

      void toggle_tool();

      GroundEffectsTool* getGroundEffectsTool();;

      void change_radius (float change);
      void setRadius(float radius);
      void setHardness(float hardness);
      void change_hardness (float change);
      void change_pressure (float change);
      void change_brush_level (float change);
      void set_pressure (float pressure);
	  void toggle_brush_level_min_max();
      void change_spray_size (float change);
      void change_spray_pressure (float change);

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider();;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getInnerRadiusSlider();;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider();;
      QDial* getMaskOrientationDial();;

      void paint (World* world, glm::vec3 const& pos, float dt, scoped_blp_texture_reference texture);      

      Brush const& texture_brush() const;

      float alpha_target() const;

      current_texture* _current_texture;

      texture_swapper* const texture_swap_tool();

      QSize sizeHint() const override;

      Noggit::Ui::Tools::ImageMaskSelector* getImageMaskSelector();;
      QImage* getMaskImage();
      texturing_mode getTexturingMode() const;;
      void updateMaskImage();

      QJsonObject toJSON();
      void fromJSON(QJsonObject const& json);

      QPushButton* const heightmappingApplyGlobalButton();
      QPushButton* const heightmappingApplyAdtButton();
      texture_heightmapping_data& getCurrentHeightMappingSetting();
    signals:
      void texturePaletteToggled();

    private:
      void change_tex_flag(World* world, glm::vec3 const& pos, bool add, scoped_blp_texture_reference texture);

      // slider functions
      void update_brush_hardness();
      void set_radius (float radius);
      void update_spray_brush();

      Brush _texture_brush;
      Brush _inner_brush;
      Brush _spray_brush;

      int _brush_level;
      bool _show_unpaintable_chunks;

      int* _heightinfo_group;

      float _spray_size;
      float _spray_pressure;

      BoolToggleProperty _anim_prop;
      unsigned_int_property _anim_speed_prop;
      unsigned_int_property _anim_rotation_prop;
      BoolToggleProperty _overbright_prop;

      texturing_mode _texturing_mode; // use getTexturingMode() to check for ground effect mode
      texture_heightmapping_data textureHeightmappingData;

    private:
      OpacitySlider* _brush_level_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _hardness_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _radius_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _pressure_slider;
      QSpinBox* _brush_level_spin;

      QCheckBox* _show_unpaintable_chunks_cb;

      QGroupBox* _spray_mode_group;
      QWidget* _spray_content;
      QCheckBox* _inner_radius_cb;
      QSlider* _spray_size_slider;
      QSlider* _spray_pressure_slider;
      QDoubleSpinBox* _spray_size_spin;
      QDoubleSpinBox* _spray_pressure_spin;

      QGroupBox* _anim_group;

      texture_swapper* _texture_switcher;

      QGroupBox* _heightmapping_group;
      QPushButton* _heightmapping_apply_global_btn;
      QPushButton* _heightmapping_apply_adt_btn;

      GroundEffectsTool* _ground_effect_tool;

      Noggit::Ui::Tools::ImageMaskSelector* _image_mask_group;

      QTabWidget* tabs;

      QImage _mask_image;
      MapView* _map_view;
    };
  }
}
