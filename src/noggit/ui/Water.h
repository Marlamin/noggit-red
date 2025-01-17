// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/BoolToggleProperty.hpp>
#include <noggit/TileIndex.hpp>

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class World;
class QComboBox;
class QRadioButton;
class QButtonGroup;

namespace Noggit
{
  struct unsigned_int_property;

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

      float brushRadius() const;
      float angle() const;
      float orientation() const;
      bool angled_mode() const;
      bool use_ref_pos() const;
      glm::vec3 ref_pos() const;

      QSize sizeHint() const override;

    signals:
      void regenerate_water_opacity (float factor);
      void crop_water();

    private:
      static constexpr float RIVER_OPACITY_VALUE = 0.0337f;
      static constexpr float OCEAN_OPACITY_VALUE = 0.007f;

      float get_opacity_factor() const;

      int _liquid_id;
      liquid_basic_types _liquid_type;
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
