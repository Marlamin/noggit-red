// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/TextureManager.h>

#include <QtWidgets/QWidget>
#include <optional>

class World;
class MapView;

class QCheckBox;
class QDoubleSpinBox;
class QGroupBox;
class QSlider;

namespace Noggit
{
  namespace Ui
  {
    class current_texture;

    class texture_swapper : public QWidget
    {
    public:
      texture_swapper ( QWidget* parent
                      , const glm::vec3* camera_pos
                      , MapView* map_view
                      );

      std::optional<scoped_blp_texture_reference> const& texture_to_swap() const;

      float radius() const;

      bool entireChunk() const;

      bool entireTile() const;

      void change_radius(float change);

      bool brush_mode() const;

      void toggle_brush_mode();

      void set_texture(std::string const& filename);

      current_texture* const texture_display();

    private:
      std::optional<scoped_blp_texture_reference> _texture_to_swap;
      float _radius;

    private:
      current_texture* _texture_to_swap_display;

      QGroupBox* _brush_mode_group;
      QSlider* _radius_slider;
      QCheckBox* _swap_entire_chunk;
      QCheckBox* _swap_entire_tile;
      QDoubleSpinBox* _radius_spin;
      World* _world;
    };
  }
}
