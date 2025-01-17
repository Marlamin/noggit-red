#ifndef NOGGIT_ModelView_HPP
#define NOGGIT_ModelView_HPP

#include <QWidget>
#include <QPointF>
#include <QElapsedTimer>
#include <QTimer>
#include <QStringList>
#include <noggit/ui/tools/PreviewRenderer/PreviewRenderer.hpp>

class QWheelEvent;
class QMouseEvent;
class QFocusEvent;
class QKeyEvent;

namespace Noggit
{
  namespace Ui::Tools::AssetBrowser
  {

  class ModelViewer : public PreviewRenderer
    {
      Q_OBJECT

    public:
      explicit ModelViewer(QWidget* parent = nullptr
          , Noggit::NoggitRenderContext context = Noggit::NoggitRenderContext::ASSET_BROWSER);

      void setModel(std::string const& filename) override;
      void setMoveSensitivity(float s);;
      float getMoveSensitivity() const;;
      QStringList getDoodadSetNames(std::string const& filename);
      void setActiveDoodadSet(std::string const& filename, std::string const& doodadset_name);
      std::string& getLastSelectedModel();;

      bool hasHeightForWidth() const override;;
      int heightForWidth(int w) const override;;

      ~ModelViewer() override;


    signals:
      void resized();
      void sensitivity_changed();
      void model_set(std::string const& filename);
      void gl_data_unloaded();

    protected:

      QTimer _update_every_event_loop;
      QPointF _last_mouse_pos;
      float moving, strafing, updown, mousedir, turn, lookat;
      bool look;
      std::string _last_selected_model;

      QElapsedTimer _startup_time;
      qreal _last_update = 0.f;
      bool _needs_redraw = false;
      float _move_sensitivity = 0.5f;

      QMetaObject::Connection _gl_guard_connection;

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

    private:
        std::array<Qt::Key, 6> _inputs = { Qt::Key_W, Qt::Key_S, Qt::Key_D, Qt::Key_A, Qt::Key_Q, Qt::Key_E };
        void checkInputsSettings();
    };
  }
}




#endif //NOGGIT_ModelView_HPP
