// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QWidget>
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

      const World* world (World* const world_);
      const World* world() const;

      const bool& draw_skies (const bool& draw_skies_);
      const bool& draw_skies() const;

      const bool& draw_boundaries (const bool& draw_boundaries_);
      const bool& draw_boundaries() const;

      const std::vector<char>* use_selection (std::vector<char>* selection_);
      const std::vector<char>* selection() const;

      void camera (Noggit::Camera* camera);
      void set_resizeable(bool state);;

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
      std::vector<char>* _selected_tiles;

      bool _draw_skies;
      bool _draw_camera;
      bool _draw_boundaries;
      bool _resizeable;

      bool _use_selection = false;
      bool _is_selecting = false;
    };
  }
}
