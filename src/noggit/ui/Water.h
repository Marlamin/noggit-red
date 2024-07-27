// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/BoolToggleProperty.hpp>
#include <noggit/TileIndex.hpp>
#include <noggit/ui/Checkbox.hpp>
#include <noggit/unsigned_int_property.hpp>

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class World;
class QComboBox;

namespace Noggit
{
  namespace Ui
  {
    class water : public QWidget
    {
      Q_OBJECT

    public:
      water ( unsigned_int_property* current_layer
            , BoolToggleProperty* display_all_layers
            , QWidget* parent = nullptr
            );

      void updatePos(TileIndex const& newTile);
      void updateData();

      void changeWaterType(int waterint);

      void paintLiquid (World*, glm::vec3 const& pos, bool add);

      void changeRadius(float change);
      void setRadius(float radius);
      void changeOrientation(float change);
      void changeAngle(float change);
      void change_height(float change);

      void lockPos(glm::vec3 const& cursor_pos);
      void toggle_lock();
      void toggle_angled_mode();

      float brushRadius() const { return _radius; }
      float angle() const { return _angle; }
      float orientation() const { return _orientation; }
      bool angled_mode() const { return _angled_mode.get(); }
      bool use_ref_pos() const { return _locked.get(); }
      glm::vec3 ref_pos() const { return _lock_pos; }

      QSize sizeHint() const override;

    signals:
      void regenerate_water_opacity (float factor);
      void crop_water();

    private:
      float get_opacity_factor() const;

      int _liquid_id;
      float _radius;

      float _angle;
      float _orientation;

      BoolToggleProperty _locked;
      BoolToggleProperty _angled_mode;

      BoolToggleProperty _override_liquid_id;
      BoolToggleProperty _override_height;

      int _opacity_mode;
      float _custom_opacity_factor;

      glm::vec3 _lock_pos;

      QDoubleSpinBox* _radius_spin;
      QDoubleSpinBox* _angle_spin;
      QDoubleSpinBox* _orientation_spin;

      QDoubleSpinBox* _x_spin;
      QDoubleSpinBox* _z_spin;
      QDoubleSpinBox* _h_spin;

      QRadioButton* river_button;
      QRadioButton* ocean_button;
      QRadioButton* custom_button;
      QButtonGroup* transparency_toggle;

      QComboBox* waterType;
      QSpinBox* waterLayer;

      TileIndex tile;
    };
  }
}
