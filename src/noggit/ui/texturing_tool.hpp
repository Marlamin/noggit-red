// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/BoolToggleProperty.hpp>
#include <noggit/Brush.h>
#include <noggit/TextureManager.h>
#include <noggit/unsigned_int_property.hpp>
#include <noggit/DBCFile.h>
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <noggit/ui/tools/UiCommon/ImageMaskSelector.hpp>
#include <noggit/ui/widget.hpp>
#include <noggit/ui/GroundEffectsTool.hpp>
#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>
#include <noggit/MapView.h>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QJsonObject>

class World;
class MapView;
class DBCFile::Record;

inline constexpr const char* STRING_EMPTY_DISPLAY = "-NONE-";

namespace Noggit
{
  namespace Ui
  {
    class CheckBox;
    class current_texture;
    class texture_swapper;


    class OpacitySlider : public QSlider{
    public:
        OpacitySlider(Qt::Orientation orientation, QWidget * parent = nullptr)
            : QSlider(orientation, parent) {
            setFixedWidth(35);
        }
    
    protected:
        void paintEvent(QPaintEvent * event) override {
            // QSlider::paintEvent(event);
    
            // chat-gpt code, can probably be improved...

            QPainter p(this);
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
            if (tickPosition() != NoTicks)
                opt.subControls |= QStyle::SC_SliderTickmarks;
            if (isSliderDown()) {
                opt.activeSubControls = QStyle::SC_SliderHandle;
                opt.state |= QStyle::State_Sunken;
            }
            else {
                opt.activeSubControls = QStyle::SC_None;
            }

            // Draw the groove with a linear gradient
            QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
            grooveRect.setLeft((width() - 35) / 2);
            grooveRect.setRight((width() + 35) / 2);
            QLinearGradient gradient(grooveRect.topLeft(), grooveRect.bottomLeft());
            gradient.setColorAt(0, Qt::black);
            gradient.setColorAt(1, Qt::white);
            p.fillRect(grooveRect.adjusted(0, 0, -1, -1), gradient);

            // Draw the handle with red color
            QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
            handleRect.setLeft((width() - 35) / 2);
            handleRect.setRight((width() + 35) / 2);
            handleRect.setHeight(5); // Set handle height to 5
            p.setBrush(Qt::red);
            p.setPen(Qt::NoPen);
            p.drawRect(handleRect);

            // Draw the ticks if needed
            if (tickPosition() != NoTicks) {
                opt.subControls = QStyle::SC_SliderTickmarks;
                style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
            }

            // QSlider::paintEvent() source code :
            /*
            Q_D(QSlider);
            QPainter p(this);
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
            if (d->tickPosition != NoTicks)
                opt.subControls |= QStyle::SC_SliderTickmarks;
            if (d->pressedControl) {
                opt.activeSubControls = d->pressedControl;
                opt.state |= QStyle::State_Sunken;
            }
            else {
                opt.activeSubControls = d->hoverControl;
            }
            style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
            */
        }
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

      GroundEffectsTool* getGroundEffectsTool() 
      { 
          return _ground_effect_tool; 
      };

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

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider() { return _radius_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getInnerRadiusSlider() { return _hardness_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider() { return _pressure_slider; };
      QDial* getMaskOrientationDial() { return _image_mask_group->getMaskOrientationDial(); };

      void paint (World* world, glm::vec3 const& pos, float dt, scoped_blp_texture_reference texture);      

      Brush const& texture_brush() const
      {
        return _texture_brush;
      }

      float alpha_target() const
      {
        return static_cast<float>(_brush_level);
      }

      current_texture* _current_texture;

      texture_swapper* const texture_swap_tool() { return _texture_switcher; }

      QSize sizeHint() const override;

      Noggit::Ui::Tools::ImageMaskSelector* getImageMaskSelector() { return _image_mask_group; };
      QImage* getMaskImage() { return &_mask_image; }
      inline texturing_mode getTexturingMode() const
      { 
          if (_ground_effect_tool->isVisible())
              return texturing_mode::ground_effect;
          else
            return _texturing_mode; 
      };
      void updateMaskImage();

      QJsonObject toJSON();
      void fromJSON(QJsonObject const& json);

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

      float _spray_size;
      float _spray_pressure;

      BoolToggleProperty _anim_prop;
      unsigned_int_property _anim_speed_prop;
      unsigned_int_property _anim_rotation_prop;
      BoolToggleProperty _overbright_prop;

      texturing_mode _texturing_mode; // use getTexturingMode() to check for ground effect mode

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

      GroundEffectsTool* _ground_effect_tool;

      Noggit::Ui::Tools::ImageMaskSelector* _image_mask_group;

      QTabWidget* tabs;

      QImage _mask_image;
      MapView* _map_view;
    };
  }
}
