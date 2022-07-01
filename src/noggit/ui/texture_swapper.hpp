// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/ui/CurrentTexture.h>
#include <noggit/TextureManager.h>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <optional>

class World;
class MapView;

namespace Noggit
{
  namespace Ui
  {
    class texture_swapper : public QWidget
    {
    public:
      texture_swapper ( QWidget* parent
                      , const glm::vec3* camera_pos
                      , MapView* map_view
                      );

      std::optional<scoped_blp_texture_reference> const& texture_to_swap() const
      {
        return _texture_to_swap;
      }

      float radius() const
      {
        return _radius;
      }

      bool entireChunk() const
      {
          return _swap_entire_chunk->isChecked();
      }

      void change_radius(float change);

      bool brush_mode() const
      {
        return _brush_mode_group->isChecked();
      }

      void toggle_brush_mode()
      {
        _brush_mode_group->setChecked(!_brush_mode_group->isChecked());
      }

      void set_texture(std::string const& filename);

      current_texture* const texture_display() { return _texture_to_swap_display; }

    private:
      std::optional<scoped_blp_texture_reference> _texture_to_swap;
      float _radius;

    private:
      current_texture* _texture_to_swap_display;

      QGroupBox* _brush_mode_group;
      QSlider* _radius_slider;
      QCheckBox* _swap_entire_chunk;
      QDoubleSpinBox* _radius_spin;
      World* _world;
    };
  }
}
