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

ModelViewer::ModelViewer(QWidget *parent)
    : QOpenGLWidget(parent)
    , _camera (math::vector_3d(0.0f, 0.0f, 0.0f), math::degrees(0.0f), math::degrees(0.0f))
    , _settings (new QSettings (this))

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

void ModelViewer::setModel(std::string const &filename)
{
  opengl::context::scoped_setter const _ (::gl, context());
  makeCurrent();

  // remove old instance
  if (_model_instance.which() == eEntry_WMO)
  {
    delete boost::get<selected_wmo_type>(_model_instance);
  }
  else
  {
    delete boost::get<selected_model_type>(_model_instance);
  }

  // add new model instance
  QString q_filename = QString(filename.c_str());

  if (q_filename.endsWith(".wmo"))
  {
    auto instance = new WMOInstance(filename, noggit::NoggitRenderContext::ASSET_BROWSER);
    _model_instance = instance;
    instance->wmo->wait_until_loaded();
    instance->recalcExtents();

  }
  else if (q_filename.endsWith(".m2"))
  {
    auto instance = new ModelInstance(filename, noggit::NoggitRenderContext::ASSET_BROWSER);
    _model_instance = instance;
    instance->model->wait_until_loaded();
    instance->recalcExtents();
  }
  else
  {
    throw std::logic_error("Asset browser only supports viewing M2 and WMO for now.");
  }

  resetCamera();
}


void ModelViewer::resetCamera()
{
  float radius = 0.f;

  if (_model_instance.which() == eEntry_WMO)
  {
    WMOInstance* wmo = boost::get<selected_wmo_type>(_model_instance);
    auto bb_center = (wmo->extents[0] + wmo->extents[1]) / 2;
    radius = (bb_center - wmo->extents[0]).length();

  }
  else
  {
    ModelInstance* model = boost::get<selected_model_type>(_model_instance);
    auto extents = model->extents();

    auto bb_center = (extents[0] + extents[1]) / 2;
    radius = (bb_center - extents[0]).length();
  }

  float distance_factor = abs( aspect_ratio() * radius / sin(_camera.fov()) / 2);
  _camera.position = {0.f, 0.f, 0.f};
  _camera.move_forward_factor(-1.f, distance_factor);

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

void ModelViewer::draw()
{

  opengl::context::scoped_setter const _ (::gl, context());
  makeCurrent();

  float culldistance = 10000000;
  bool draw_doodads_wmo = true;

  math::matrix_4x4 const mvp(model_view().transposed() * projection().transposed());
  math::frustum const frustum (mvp);

  if (!_m2_program)
  {
    setModel("world/wmo/azeroth/buildings/human_farm/farm.wmo");
    _m2_program.reset
        ( new opengl::program
              { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs") }
                  , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
              }
        );
  }
  if (!_m2_instanced_program)
  {
    _m2_instanced_program.reset
        ( new opengl::program
              { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs", {"instanced"}) }
                  , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
              }
        );
  }

  if (!_m2_ribbons_program)
  {
    _m2_ribbons_program.reset
        ( new opengl::program
              { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("ribbon_vs") }
                  , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("ribbon_fs") }
              }
        );
  }
  if (!_m2_particles_program)
  {
    _m2_particles_program.reset
        ( new opengl::program
              { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("particle_vs") }
                  , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("particle_fs") }
              }
        );
  }

  if (!_wmo_program)
  {
    _wmo_program.reset
        ( new opengl::program
              { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("wmo_vs") }
                  , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("wmo_fs") }
              }
        );
  }

  if (!_liquid_render)
  {
    _liquid_render.emplace();
  }

  gl.enable(GL_DEPTH_TEST);
  gl.depthFunc(GL_LEQUAL);
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // draw WMOs
  std::unordered_map<std::string, std::vector<ModelInstance*>> _wmo_doodads;

  if (_model_instance.which() == eEntry_WMO)
  {
    // set anim time only once per frame
    {
      opengl::scoped::use_program water_shader {_liquid_render->shader_program()};
      water_shader.uniform("animtime", 0 / 2880.f);

      water_shader.uniform("model_view", model_view().transposed());
      water_shader.uniform("projection", projection().transposed());

      math::vector_4d ocean_color_light(math::vector_3d(1.0f, 1.0f, 1.0f), 1.f);
      math::vector_4d ocean_color_dark(math::vector_3d(1.0f, 1.0f, 1.0f), 1.f);
      math::vector_4d river_color_light(math::vector_3d(1.0f, 1.0f, 1.0f), 1.f);
      math::vector_4d river_color_dark(math::vector_3d(1.0f, 1.0f, 1.0f), 1.f);

      water_shader.uniform("ocean_color_light", ocean_color_light);
      water_shader.uniform("ocean_color_dark", ocean_color_dark);
      water_shader.uniform("river_color_light", river_color_light);
      water_shader.uniform("river_color_dark", river_color_dark);
      water_shader.uniform("use_transform", 1);
    }

    {
      opengl::scoped::use_program wmo_program{*_wmo_program.get()};

      wmo_program.uniform("model_view", model_view().transposed());
      wmo_program.uniform("projection", projection().transposed());
      wmo_program.uniform("tex1", 0);
      wmo_program.uniform("tex2", 1);

      wmo_program.uniform("draw_fog", 0);

      wmo_program.uniform("exterior_light_dir", math::vector_3d(0.0f, 1.0f, 0.0f));
      wmo_program.uniform("exterior_diffuse_color", math::vector_3d(1.0f, 1.0f, 1.0f));
      wmo_program.uniform("exterior_ambient_color", math::vector_3d(1.0f, 1.0f, 1.0f));

      auto wmo_instance = boost::get<selected_wmo_type>(_model_instance);

      wmo_instance->draw(
          wmo_program, model_view().transposed(), projection().transposed(), frustum, culldistance,
          math::vector_3d(0.0f, 0.0f, 0.0f), false, false // doodads
          , false, _liquid_render.get(), std::vector<selection_type>(), 0, false, display_mode::in_3D
      );

      gl.enable(GL_BLEND);
      gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      gl.enable(GL_CULL_FACE);

      if (draw_doodads_wmo)
      {
        for (auto &doodad : wmo_instance->get_visible_doodads(frustum, culldistance, _camera.position, false,
                                                              display_mode::in_3D))
        {
          _wmo_doodads[doodad->model->filename].push_back(doodad);
        }
      }

    }

  }

  // draw M2
  std::unordered_map<Model*, std::size_t> model_with_particles;
  std::unordered_map<Model*, std::size_t> model_boxes_to_draw;

  {
    opengl::scoped::use_program m2_shader {*_m2_instanced_program.get()};

    m2_shader.uniform("model_view", model_view().transposed());
    m2_shader.uniform("projection", projection().transposed());
    m2_shader.uniform("tex1", 0);
    m2_shader.uniform("tex2", 1);
    m2_shader.uniform("draw_fog", 0);

    m2_shader.uniform("light_dir", math::vector_3d(0.0f, 1.0f, 0.0f));
    m2_shader.uniform("diffuse_color", math::vector_3d(1.0f, 1.0f, 1.0f));
    m2_shader.uniform("ambient_color", math::vector_3d(1.0f, 1.0f, 1.0f));

    if (_model_instance.which() == eEntry_Model)
    {
      auto model_instance = boost::get<selected_model_type>(_model_instance);

      model_instance->model->draw(
          model_view().transposed()
          , std::vector<ModelInstance*>{model_instance}
          , m2_shader
          , frustum
          , culldistance
          , _camera.position
          , false
          , 0
          , false
          , false
          , model_with_particles
          , model_boxes_to_draw
          , display_mode::in_3D
      );
    }

    if (draw_doodads_wmo)
    {
      for (auto& it : _wmo_doodads)
      {
        it.second[0]->model->draw(
            model_view().transposed()
            , it.second
            , m2_shader
            , frustum
            , culldistance
            , _camera.position
            , false
            , 0
            , false
            , false
            , model_with_particles
            , model_boxes_to_draw
            , display_mode::in_3D
        );
      }
    }
  }

  gl.bindVertexArray(0);
  gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

math::matrix_4x4 ModelViewer::model_view() const
{
  return _camera.look_at_matrix();
}

math::matrix_4x4 ModelViewer::projection() const
{
  float far_z = _settings->value("farZ", 2048).toFloat();
  return math::perspective(_camera.fov(), aspect_ratio(), 1.f, far_z);
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
