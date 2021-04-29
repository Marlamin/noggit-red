// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/tool_enums.hpp>
#include <noggit/Red/UiCommon/ExtendedSlider.hpp>
#include <math/vector_3d.hpp>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

class World;
namespace noggit
{
  namespace ui
  {
    class flatten_blur_tool : public QWidget
    {
    public:
      flatten_blur_tool(QWidget* parent = nullptr);

      void flatten (World* world, math::vector_3d const& cursor_pos, float dt);
      void blur (World* world, math::vector_3d const& cursor_pos, float dt);

      void nextFlattenType();
      void nextFlattenMode();
      void toggleFlattenAngle();
      void toggleFlattenLock();
      void lockPos (math::vector_3d const& cursor_pos);

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
      math::vector_3d ref_pos() const { return _lock_pos; }

      QSize sizeHint() const override;

    private:

      float _angle;
      float _orientation;

      math::vector_3d _lock_pos;

      int _flatten_type;
      flatten_mode _flatten_mode;

    private:
      QButtonGroup* _type_button_box;
      noggit::Red::UiCommon::ExtendedSlider* _radius_slider;
      noggit::Red::UiCommon::ExtendedSlider* _speed_slider;

      QGroupBox* _angle_group;
      QSlider* _angle_slider;
      QDial* _orientation_dial;

      QGroupBox* _lock_group;
      QDoubleSpinBox* _lock_x;
      QDoubleSpinBox* _lock_z;
      QDoubleSpinBox* _lock_h;

      QCheckBox* _lock_up_checkbox;
      QCheckBox* _lock_down_checkbox;
    };
  }
}
