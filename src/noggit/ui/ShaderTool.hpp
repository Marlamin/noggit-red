// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>

#include <QJsonObject>

namespace color_widgets
{
  class ColorListWidget;
  class ColorSelector;
  class ColorWheel;
  class GradientSlider;
  class HueSlider;
}

class World;
class MapView;

class QCheckBox;
class QDial;
class QSpinBox;

namespace Noggit
{
  namespace Ui
  {
    namespace Tools
    {
      namespace UiCommon
      {
        class ExtendedSlider;
      }

      class ImageMaskSelector;
    }

    class ShaderTool : public QWidget
    {
      Q_OBJECT
    public:
      ShaderTool(MapView* map_view, QWidget* parent = nullptr);

      void changeShader (World*, glm::vec3 const& pos, float dt, bool add);
      void pickColor(World* world, glm::vec3 const& pos);
      void addColorToPalette();

      void changeRadius(float change);
      void setRadius(float radius);
      void changeSpeed(float change);
      void setSpeed(float speed);

      float brushRadius() const;

      QSize sizeHint() const override;

      Noggit::Ui::Tools::ImageMaskSelector* getImageMaskSelector();;
      QImage* getMaskImage();
      void updateMaskImage();

      glm::vec4& shaderColor();;

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider();;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider();;
      QDial* getMaskOrientationDial();;

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
