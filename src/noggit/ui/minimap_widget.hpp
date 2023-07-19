// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QWidget>
#include <array>
#include <glm/vec3.hpp>

namespace math
{
  struct vector_3d;
}
class World;

//! \todo add adt coordinates/name on mouseover
namespace Noggit
{
  class Camera;

  namespace Ui
  {
    //! \todo Make this a fixed square somehow.
    class minimap_widget : public QWidget
    {
      Q_OBJECT

    public:
      minimap_widget (QWidget* parent = nullptr);

      virtual QSize sizeHint() const override;

      inline const World* world (World* const world_)
        { _world = world_; update(); return _world; }
      inline const World* world() const { return _world; }

      inline const bool& draw_skies (const bool& draw_skies_)
        { _draw_skies = draw_skies_; update(); return _draw_skies; }
      inline const bool& draw_skies() const { return _draw_skies; }

      inline const bool& draw_boundaries (const bool& draw_boundaries_)
        { _draw_boundaries = draw_boundaries_; update(); return _draw_boundaries; }
      inline const bool& draw_boundaries() const { return _draw_boundaries; }

      inline const std::array<bool, 4096>* use_selection (std::array<bool, 4096>* selection_)
      { _use_selection = selection_; _selected_tiles = selection_; update(); return _selected_tiles; }
      inline const std::array<bool, 4096>* selection() const { return _selected_tiles; }

      inline void camera (Noggit::Camera* camera) { _camera = camera; }
      void set_resizeable(bool state) { _resizeable = state; };

    protected:
      virtual void paintEvent (QPaintEvent* paint_event) override;
      virtual void mouseDoubleClickEvent (QMouseEvent*) override;
      virtual void mouseMoveEvent(QMouseEvent*) override;
      virtual void mousePressEvent(QMouseEvent* event) override;
      virtual void mouseReleaseEvent(QMouseEvent* event) override;
      virtual void wheelEvent(QWheelEvent* event) override;

      QPoint locateTile(QMouseEvent* event);

    signals:
      void map_clicked(const glm::vec3&);
      void tile_clicked(const QPoint&);
      void reset_selection();

    private:
      World* _world;
      Noggit::Camera* _camera;
      std::array<bool, 4096>* _selected_tiles;

      bool _draw_skies;
      bool _draw_camera;
      bool _draw_boundaries;
      bool _resizeable;

      bool _use_selection = false;
      bool _is_selecting = false;
    };
  }
}
