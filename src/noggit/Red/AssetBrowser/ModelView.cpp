#include "ModelView.hpp"
#include <opengl/scoped.hpp>
#include <math/projection.hpp>
#include <noggit/Selection.h>
#include <noggit/tool_enums.hpp>
#include <noggit/ContextObject.hpp>

#include <vector>
#include <cmath>
#include <stdexcept>


using namespace noggit::Red::AssetBrowser;


ModelViewer::ModelViewer(QWidget* parent)
 : PreviewRenderer(0, 0, noggit::NoggitRenderContext::ASSET_BROWSER, parent)
{
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking (true);

  _startup_time.start();
  look = false;
  moving = strafing = updown = lookat = turn = 0.0f;
  mousedir = -1.0f;

  _move_sensitivity = _settings->value("assetBrowser/move_sensitivity", 15.0f).toFloat() / 30.0f;

  _update_every_event_loop.start (0);
  connect (&_update_every_event_loop, &QTimer::timeout, [this] { update(); });
}

void ModelViewer::initializeGL()
{
  opengl::context::scoped_setter const _ (::gl, context());
  gl.viewport(0.0f, 0.0f, width(), height());
  gl.clearColor (0.5f, 0.5f, 0.5f, 1.0f);
  emit resized();
}

void ModelViewer::paintGL()
{
  const qreal now(_startup_time.elapsed() / 1000.0);

  opengl::context::scoped_setter const _ (::gl, context());
  makeCurrent();

  gl.clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  tick(now - _last_update);

  _last_update = now;

  draw();
}

void ModelViewer::resizeGL(int w, int h)
{
  opengl::context::scoped_setter const _ (::gl, context());
  gl.viewport(0.0f, 0.0f, w, h);
  emit resized();
}

void ModelViewer::tick(float dt)
{
  if (turn)
  {
    _camera.add_to_yaw(math::degrees(turn));
  }
  if (lookat)
  {
    _camera.add_to_pitch(math::degrees(lookat));
  }
  if (moving)
  {
    _camera.move_forward(moving, dt);
  }
  if (strafing)
  {
    _camera.move_horizontal(strafing, dt);
  }
  if (updown)
  {
    _camera.move_vertical(updown, dt);
  }
}

void ModelViewer::setModel(std::string const& filename)
{
  opengl::context::scoped_setter const _ (::gl, context());
  makeCurrent();
  PreviewRenderer::setModel(filename);
}

float ModelViewer::aspect_ratio() const
{
  return float (width()) / float (height());
}

void ModelViewer::mouseMoveEvent(QMouseEvent* event)
{
  QLineF const relative_movement (_last_mouse_pos, event->pos());

  if (look)
  {
    _camera.add_to_yaw(math::degrees(relative_movement.dx() / 20.0f));
    _camera.add_to_pitch(math::degrees(mousedir * relative_movement.dy() / 20.0f));
  }

  _last_mouse_pos = event->pos();
}

void ModelViewer::mousePressEvent(QMouseEvent* event)
{
  look = true;
}

void ModelViewer::mouseReleaseEvent(QMouseEvent* event)
{
  look = false;
}

void ModelViewer::wheelEvent(QWheelEvent* event)
{
  if (event->angleDelta().y() > 0)
  {
    _move_sensitivity = std::min(_move_sensitivity + 0.5f / 30.0f, 1.0f);
  }
  else
  {
    _move_sensitivity = std::max(_move_sensitivity - 0.5f / 30.0f, 1.0f / 30.0f);
  }

  emit sensitivity_changed();
}

void ModelViewer::keyReleaseEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_W || event->key() == Qt::Key_S)
  {
    moving = 0.0f;
  }

  if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
  {
    lookat = 0.0f;
  }

  if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Left)
  {
    turn  = 0.0f;
  }

  if (event->key() == Qt::Key_D || event->key() == Qt::Key_A)
  {
    strafing  = 0.0f;
  }

  if (event->key() == Qt::Key_Q || event->key() == Qt::Key_E)
  {
    updown  = 0.0f;
  }

}

void ModelViewer::focusOutEvent(QFocusEvent* event)
{
  //moving = 0.0f;
  //lookat = 0.0f;
 // turn = 0.0f;
  //strafing = 0.0f;
 // updown = 0.0f;
}

void ModelViewer::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_W)
  {
    moving = _move_sensitivity;
  }
  if (event->key() == Qt::Key_S)
  {
    moving = -_move_sensitivity;
  }

  if (event->key() == Qt::Key_Up)
  {
    lookat = _move_sensitivity;
  }
  if (event->key() == Qt::Key_Down)
  {
    lookat = -_move_sensitivity;
  }

  if (event->key() == Qt::Key_Right)
  {
    turn = _move_sensitivity;
  }
  if (event->key() == Qt::Key_Left)
  {
    turn = -_move_sensitivity;
  }

  if (event->key() == Qt::Key_D)
  {
    strafing = _move_sensitivity;
  }
  if (event->key() == Qt::Key_A)
  {
    strafing = -_move_sensitivity;
  }

  if (event->key() == Qt::Key_Q)
  {
    updown = _move_sensitivity;
  }
  if (event->key() == Qt::Key_E)
  {
    updown = -_move_sensitivity;
  }
}
