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

static const float XSENS = 15.0f;
static const float YSENS = 15.0f;

ModelViewer::ModelViewer(QWidget* parent)
 : PreviewRenderer(0, 0, noggit::NoggitRenderContext::ASSET_BROWSER, parent)
{
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking (true);

  _startup_time.start();
  look = false;
  moving = strafing = updown = lookat = turn = 0.0f;
  mousedir = -1.0f;

  _update_every_event_loop.start (0);
  connect (&_update_every_event_loop, &QTimer::timeout, [this] { update(); });
}

void ModelViewer::initializeGL()
{
  opengl::context::scoped_setter const _ (::gl, context());
  gl.viewport(0.0f, 0.0f, width(), height());
  gl.clearColor (0.7f, 0.5f, 0.5f, 1.0f);
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
    _camera.add_to_yaw(math::degrees(relative_movement.dx() / XSENS));
    _camera.add_to_pitch(math::degrees(mousedir * relative_movement.dy() / YSENS));
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
    moving = 1.0f;
  }
  if (event->key() == Qt::Key_S)
  {
    moving = -1.0f;
  }

  if (event->key() == Qt::Key_Up)
  {
    lookat = 0.75f;
  }
  if (event->key() == Qt::Key_Down)
  {
    lookat = -0.75f;
  }

  if (event->key() == Qt::Key_Right)
  {
    turn = 0.75f;
  }
  if (event->key() == Qt::Key_Left)
  {
    turn = -0.75f;
  }

  if (event->key() == Qt::Key_D)
  {
    strafing = 1.0f;
  }
  if (event->key() == Qt::Key_A)
  {
    strafing = -1.0f;
  }

  if (event->key() == Qt::Key_Q)
  {
    updown = 1.0f;
  }
  if (event->key() == Qt::Key_E)
  {
    updown = -1.0f;
  }
}
