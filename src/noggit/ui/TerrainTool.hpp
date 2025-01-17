// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <math/trig.hpp>
#include <noggit/tool_enums.hpp>

#include <QImage>
#include <QJsonObject>
#include <QtWidgets/QWidget>

class World;
class MapView;

class QButtonGroup;
class QCheckBox;
class QDial;
class QGroupBox;
class QSlider;

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

    class TerrainTool : public QWidget
    {
      Q_OBJECT

    public:
      TerrainTool(MapView* map_view, QWidget* parent = nullptr, bool stamp = false);

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
      float getSpeed () const;;

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider();;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getInnerRadiusSlider();;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider();;
      QDial* getMaskOrientationDial();;

      // vertex edit only functions
      void moveVertices (World*, float dt);
      void flattenVertices (World*);

      void changeOrientation (float change);
      void changeAngle (float change);
      void setOrientRelativeTo (World*, glm::vec3 const& pos);

      float brushRadius() const;
      float innerRadius() const;

      void storeCursorPos (glm::vec3* cursor_pos);

      Noggit::Ui::Tools::ImageMaskSelector* getImageMaskSelector();;

      QImage* getMaskImage();;

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
      QCheckBox* _snap_m2_objects_chkbox;
      QCheckBox* _snap_wmo_objects_chkbox;

      QSlider* _angle_slider;
      QDial* _orientation_dial;
      MapView* _map_view;

      QImage _mask_image;
    };
  }
}
