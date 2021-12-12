// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <math/trig.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <noggit/ui/tools/UiCommon/ImageMaskSelector.hpp>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QImage>
#include <QJsonObject>

class World;
class MapView;

namespace Noggit
{

  namespace Ui
  {
    class terrain_tool : public QWidget
    {
      Q_OBJECT

    public:
      terrain_tool(MapView* map_view, QWidget* parent = nullptr, bool stamp = false);

      void changeTerrain (World*, glm::vec3 const& pos, float dt);

      void nextType();
      void changeRadius(float change);
      void changeInnerRadius(float change);
      void changeSpeed(float change);

      void setRadius (float radius);
      void setInnerRadius (float radius);
      void setOrientation (float orientation);
      void setAngle (float angle);
      void setSpeed (float speed);
      float getSpeed () { return _speed_slider->value(); };

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider() { return _radius_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getInnerRadiusSlider() { return _inner_radius_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider() { return _speed_slider; };
      QDial* getMaskOrientationDial() { return _image_mask_group->getMaskOrientationDial(); };

      // vertex edit only functions
      void moveVertices (World*, float dt);
      void flattenVertices (World*);

      void changeOrientation (float change);
      void changeAngle (float change);
      void setOrientRelativeTo (World*, glm::vec3 const& pos);

      float brushRadius() const { return static_cast<float>(_radius_slider->value()); }
      float innerRadius() const { return static_cast<float>(_inner_radius_slider->value());  }

      void storeCursorPos (glm::vec3* cursor_pos) { _cursor_pos = cursor_pos; }

      Noggit::Ui::Tools::ImageMaskSelector* getImageMaskSelector() { return _image_mask_group; };

      QImage* getMaskImage() { return &_mask_image; };

      QSize sizeHint() const override;

      eTerrainType _edit_type;

      QJsonObject toJSON();
      void fromJSON(QJsonObject const& json);

    signals:
      void updateVertices(int vertex_mode, math::degrees const& angle, math::degrees const& orientation);

    public slots:
      void updateMaskImage();

    private:
      void updateVertexGroup();

      math::degrees _vertex_angle;
      math::degrees _vertex_orientation;

      glm::vec3* _cursor_pos;

      int _vertex_mode;

      // UI stuff:

      QButtonGroup* _type_button_group;
      QButtonGroup* _vertex_button_group;
      QGroupBox* _speed_box;
      QGroupBox* _vertex_type_group;
      Noggit::Ui::Tools::ImageMaskSelector* _image_mask_group;

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _radius_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _inner_radius_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _speed_slider;

      QSlider* _angle_slider;
      QDial* _orientation_dial;
      MapView* _map_view;

      QImage _mask_image;
    };
  }
}
