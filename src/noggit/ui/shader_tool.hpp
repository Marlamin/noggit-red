// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_4d.hpp>
#include <noggit/Red/UiCommon/ExtendedSlider.hpp>
#include <noggit/Red/UiCommon/ImageMaskSelector.hpp>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

#include <qt-color-widgets/color_selector.hpp>
#include <qt-color-widgets/color_wheel.hpp>
#include <qt-color-widgets/hue_slider.hpp>
#include <qt-color-widgets/gradient_slider.hpp>
#include <qt-color-widgets/color_list_widget.hpp>

#include <QJsonObject>

class World;
class MapView;

namespace noggit
{
  namespace ui
  {
    class shader_tool : public QWidget
    {
      Q_OBJECT
    public:
      shader_tool(MapView* map_view, QWidget* parent = nullptr);

      void changeShader (World*, math::vector_3d const& pos, float dt, bool add);
      void pickColor(World* world, math::vector_3d const& pos);
      void addColorToPalette();

      void changeRadius(float change);
      void setRadius(float radius);
      void changeSpeed(float change);
      void setSpeed(float speed);

      float brushRadius() const { return _radius_slider->value(); }

      QSize sizeHint() const override;

      noggit::Red::ImageMaskSelector* getImageMaskSelector() { return _image_mask_group; };
      QImage* getMaskImage() { return &_mask_image; }
      void updateMaskImage();

      math::vector_4d& shaderColor() { return _color; };

      noggit::Red::UiCommon::ExtendedSlider* getRadiusSlider() { return _radius_slider; };
      noggit::Red::UiCommon::ExtendedSlider* getSpeedSlider() { return _speed_slider; };
      QDial* getMaskOrientationDial() { return _image_mask_group->getMaskOrientationDial(); };

      QJsonObject toJSON();
      void fromJSON(QJsonObject const& json);

    private:
      math::vector_4d _color;

      noggit::Red::UiCommon::ExtendedSlider* _radius_slider;
      noggit::Red::UiCommon::ExtendedSlider* _speed_slider;
      QSpinBox* _spin_hue;
      QSpinBox* _spin_saturation;
      QSpinBox* _spin_value;

      color_widgets::ColorSelector* color_picker;
      color_widgets::ColorWheel* color_wheel;
      color_widgets::HueSlider* _slide_hue;
      color_widgets::GradientSlider* _slide_saturation;
      color_widgets::GradientSlider* _slide_value;
      color_widgets::ColorListWidget* _color_palette;

      noggit::Red::ImageMaskSelector* _image_mask_group;
      QImage _mask_image;
      MapView* _map_view;

    public Q_SLOTS:
      void set_hsv();
      void update_color_widgets();

    };
  }
}
