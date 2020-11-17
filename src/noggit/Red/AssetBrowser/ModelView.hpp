#ifndef NOGGIT_ModelView_HPP
#define NOGGIT_ModelView_HPP

#include <QWidget>
#include <QOpenGLWidget>
#include <QSettings>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QPointF>
#include <QElapsedTimer>
#include <QTimer>

#include <math/matrix_4x4.hpp>
#include <math/vector_3d.hpp>
#include <noggit/camera.hpp>
#include <noggit/WMOInstance.h>
#include <noggit/Selection.h>
#include <noggit/WMO.h>
#include <noggit/Model.h>
#include <noggit/Red/PreviewRenderer/PreviewRenderer.hpp>


namespace noggit
{
  namespace Red::AssetBrowser
  {

  class ModelViewer : public PreviewRenderer
    {
      Q_OBJECT

    public:
      explicit ModelViewer(QWidget* parent = nullptr);
      void setModel(std::string const& filename) override;
      void setMoveSensitivity(float s) { _move_sensitivity = s / 30.0f; }
      float getMoveSensitivity() { return _move_sensitivity; }

    signals:
      void resized();
      void sensitivity_changed();

    private:

      QTimer _update_every_event_loop;
      QPointF _last_mouse_pos;
      float moving, strafing, updown, mousedir, turn, lookat;
      bool look;

      QElapsedTimer _startup_time;
      qreal _last_update = 0.f;
      float _move_sensitivity = 0.5f;

      void tick(float dt) override;
      float aspect_ratio() const override;

      void initializeGL() override;
      void paintGL() override;
      void resizeGL (int w, int h) override;

      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;

    };
  }
}




#endif //NOGGIT_ModelView_HPP
