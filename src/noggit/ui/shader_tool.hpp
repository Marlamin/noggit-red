// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <noggit/ui/tools/UiCommon/ImageMaskSelector.hpp>

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

namespace Noggit
{
  namespace Ui
  {
    class shader_tool : public QWidget
    {
      Q_OBJECT
    public:
      shader_tool(MapView* map_view, QWidget* parent = nullptr);

      void changeShader (World*, glm::vec3 const& pos, float dt, bool add);
      void pickColor(World* world, glm::vec3 const& pos);
      void addColorToPalette();

      void changeRadius(float change);
      void setRadius(float radius);
      void changeSpeed(float change);
      void setSpeed(float speed);

      float brushRadius() const { return _radius_slider->value(); }

      QSize sizeHint() const override;

      Noggit::Ui::Tools::ImageMaskSelector* getImageMaskSelector() { return _image_mask_group; };
      QImage* getMaskImage() { return &_mask_image; }
      void updateMaskImage();

      glm::vec4& shaderColor() { return _color; };

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider() { return _radius_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider() { return _speed_slider; };
      QDial* getMaskOrientationDial() { return _image_mask_group->getMaskOrientationDial(); };

      QJsonObject toJSON();
      void fromJSON(QJsonObject const& json);

    private:
      glm::vec4 _color;

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _radius_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _speed_slider;
      QSpinBox* _spin_hue;
      QSpinBox* _spin_saturation;
      QSpinBox* _spin_value;

      QCheckBox* _use_image_colors;

      color_widgets::ColorSelector* color_picker;
      color_widgets::ColorWheel* color_wheel;
      color_widgets::HueSlider* _slide_hue;
      color_widgets::GradientSlider* _slide_saturation;
      color_widgets::GradientSlider* _slide_value;
      color_widgets::ColorListWidget* _color_palette;

      Noggit::Ui::Tools::ImageMaskSelector* _image_mask_group;
      QImage _mask_image;
      MapView* _map_view;

    public Q_SLOTS:
      void set_hsv();
      void update_color_widgets();

    };
  }
}
