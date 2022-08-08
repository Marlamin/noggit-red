// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/tool_enums.hpp>
#include <noggit/ui/tools/UiCommon/ExtendedSlider.hpp>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QJsonObject>
#include <glm/vec3.hpp>

class World;
namespace Noggit
{
  namespace Ui
  {
    class flatten_blur_tool : public QWidget
    {
    public:
      flatten_blur_tool(QWidget* parent = nullptr);

      void flatten (World* world, glm::vec3 const& cursor_pos, float dt);
      void blur (World* world, glm::vec3 const& cursor_pos, float dt);

      void nextFlattenType();
      void toggleFlattenAngle();
      void toggleFlattenLock();
      void lockPos (glm::vec3 const& cursor_pos);

      void changeRadius(float change);
      void changeSpeed(float change);
      void changeOrientation(float change);
      void changeAngle(float change);
      void changeHeight(float change);

      void setRadius(float radius);
      void setSpeed(float speed);
      void setOrientation(float orientation);

      float brushRadius() const { return _radius_slider->value(); }
      float angle() const { return _angle; }
      float orientation() const { return _orientation; }
      bool angled_mode() const { return _angle_group->isChecked(); }
      bool use_ref_pos() const  { return _lock_group->isChecked(); }
      glm::vec3 ref_pos() const { return _lock_pos; }

      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getRadiusSlider() { return _radius_slider; };
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* getSpeedSlider() { return _speed_slider; };

      QSize sizeHint() const override;
      flatten_mode _flatten_mode;

      QJsonObject toJSON();
      void fromJSON(QJsonObject const& json);

    private:
      float _angle;
      float _orientation;

      glm::vec3 _lock_pos;

      int _flatten_type;

    private:
      QButtonGroup* _type_button_box;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _radius_slider;
      Noggit::Ui::Tools::UiCommon::ExtendedSlider* _speed_slider;

      QGroupBox* _angle_group;
      QSlider* _angle_slider;
      QDial* _orientation_dial;
      QLabel* _orientation_info;
      QLabel* _angle_info;

      QGroupBox* _lock_group;
      QDoubleSpinBox* _lock_x;
      QDoubleSpinBox* _lock_z;
      QDoubleSpinBox* _lock_h;

      QCheckBox* _lock_up_checkbox;
      QCheckBox* _lock_down_checkbox;
    };
  }
}
