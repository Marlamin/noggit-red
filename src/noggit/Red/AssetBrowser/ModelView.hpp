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


namespace noggit
{
  namespace Red::AssetBrowser
  {

    class ModelViewer : public QOpenGLWidget
    {
      Q_OBJECT

    public:
        explicit ModelViewer(QWidget* parent = nullptr);

        void resetCamera();
        void setModel(std::string const& filename);

    private:

      QTimer _update_every_event_loop;
      QPointF _last_mouse_pos;
      float moving, strafing, updown, mousedir, turn, lookat;
      bool look;

      QElapsedTimer _startup_time;
      qreal _last_update = 0.f;

      noggit::camera _camera;
      QSettings* _settings;

      std::unique_ptr<opengl::program> _m2_program;
      std::unique_ptr<opengl::program> _m2_instanced_program;
      std::unique_ptr<opengl::program> _m2_particles_program;
      std::unique_ptr<opengl::program> _m2_ribbons_program;
      std::unique_ptr<opengl::program> _m2_box_program;
      std::unique_ptr<opengl::program> _wmo_program;

      boost::optional<liquid_render> _liquid_render = boost::none;

      selection_type _model_instance;

      void tick(float dt);
      void draw();
      math::matrix_4x4 model_view() const;
      math::matrix_4x4 projection() const;
      float aspect_ratio() const;

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
