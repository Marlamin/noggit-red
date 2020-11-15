#include "PreviewRenderer.hpp"

#include <opengl/scoped.hpp>
#include <math/projection.hpp>
#include <noggit/Selection.h>
#include <noggit/tool_enums.hpp>
#include <noggit/AsyncLoader.h>

#include <vector>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <thread>
#include <chrono>

#include <QSettings>


using namespace noggit::Red;


PreviewRenderer::PreviewRenderer(int width, int height, noggit::NoggitRenderContext context, QWidget* parent)
  : QOpenGLWidget(parent)
  , _camera (math::vector_3d(0.0f, 0.0f, 0.0f), math::degrees(0.0f), math::degrees(0.0f))
  , _settings (new QSettings())
  , _width(width)
  , _height(height)
  , _context(context)
{
  _cache = {};

  opengl::context::save_current_context const context_save (::gl);
  _offscreen_context.create();

  _fmt.setSamples(1);
  _fmt.setInternalTextureFormat(GL_RGBA8);
  _fmt.setAttachment(QOpenGLFramebufferObject::Depth);

  _offscreen_surface.create();
  _offscreen_context.makeCurrent(&_offscreen_surface);

  opengl::context::scoped_setter const context_set (::gl, &_offscreen_context);
}

void PreviewRenderer::setModel(std::string const &filename)
{
  _filename = filename;
  _model_instances.clear();
  _wmo_instances.clear();

  // add new model instance
  QString q_filename = QString(filename.c_str());

  if (q_filename.endsWith(".wmo"))
  {
    auto& instance = _wmo_instances.emplace_back(filename, _context);
    instance.wmo->wait_until_loaded();
    instance.recalcExtents();

  }
  else if (q_filename.endsWith(".m2"))
  {
    auto& instance = _model_instances.emplace_back(filename, _context);
    instance.model->wait_until_loaded();
    instance.recalcExtents();
  }
  else
  {
    throw std::logic_error("Preview renderer only supports viewing M2 and WMO for now.");
  }

  resetCamera();
}

void PreviewRenderer::setModelOffscreen(std::string const& filename)
{
  opengl::context::save_current_context const context_save (::gl);
  _offscreen_context.makeCurrent(&_offscreen_surface);
  opengl::context::scoped_setter const context_set (::gl, &_offscreen_context);

  setModel(filename);
}


void PreviewRenderer::resetCamera()
{
  _camera.reset();
  float radius = 0.f;

  std::vector<math::vector_3d> extents = calcSceneExtents();
  _camera.position = (extents[0] + extents[1]) / 2.0f;
  radius = std::max((_camera.position - extents[0]).length(), (_camera.position - extents[1]).length());

  float distance_factor = abs( aspect_ratio() * radius / sin(_camera.fov()._ / 2.f));
  _camera.move_forward_factor(-1.f, 0.75f * distance_factor);

}


void PreviewRenderer::draw()
{

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

  if (!_wmo_instances.empty())
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

     for (auto& wmo_instance : _wmo_instances)
     {
       wmo_instance.draw(
           wmo_program, model_view().transposed(), projection().transposed(), frustum, culldistance,
           math::vector_3d(0.0f, 0.0f, 0.0f), false, false // doodads
           , false, _liquid_render.get(), std::vector<selection_type>(), 0, false, display_mode::in_3D
       );

       gl.enable(GL_BLEND);
       gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
       gl.enable(GL_CULL_FACE);

       if (draw_doodads_wmo)
       {
         for (auto& doodad : wmo_instance.get_visible_doodads(frustum, culldistance, _camera.position, false,
                                                               display_mode::in_3D))
         {
           _wmo_doodads[doodad->model->filename].push_back(doodad);
         }
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

    for (auto& model_instance : _model_instances)
    {
      model_instance.model->draw(
          model_view().transposed()
          , std::vector<ModelInstance*>{&model_instance}
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

math::matrix_4x4 PreviewRenderer::model_view() const
{
  return _camera.look_at_matrix();
}

math::matrix_4x4 PreviewRenderer::projection() const
{
  float far_z = _settings->value("farZ", 2048).toFloat();
  return math::perspective(_camera.fov(), aspect_ratio(), 1.f, far_z);
}

float PreviewRenderer::aspect_ratio() const
{
  return static_cast<float>(_width) / static_cast<float>(_height);
}

std::vector<math::vector_3d> PreviewRenderer::calcSceneExtents()
{
  math::vector_3d min = {std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max()};

  math::vector_3d max = {std::numeric_limits<float>::min(),
                         std::numeric_limits<float>::min(),
                         std::numeric_limits<float>::min()};

  for (auto instance : _model_instances)
  {
    for (int i = 0; i < 3; ++i)
    {
      min[i] = std::min(instance.extents()[0][i], min[i]);
      max[i] = std::max(instance.extents()[1][i], max[i]);
    }
  }

  for (auto instance : _wmo_instances)
  {
    for (int i = 0; i < 3; ++i)
    {
      min[i] = std::min(instance.extents[0][i], min[i]);
      max[i] = std::max(instance.extents[1][i], max[i]);
    }
  }

  return std::move(std::vector<math::vector_3d>{min, max});
}

QPixmap* PreviewRenderer::renderToPixmap()
{
  std::tuple<std::string, int, int> const curEntry{_filename, _width, _height};
  auto it{_cache.find(curEntry)};

  if(it != _cache.end())
    return &it->second;

  opengl::context::save_current_context const context_save (::gl);

  _offscreen_context.makeCurrent(&_offscreen_surface);

  opengl::context::scoped_setter const context_set (::gl, &_offscreen_context);

  QOpenGLFramebufferObject pixel_buffer(_width, _height, _fmt);
  pixel_buffer.bind();

  gl.viewport(0, 0, _width, _height);
  gl.clearColor(0.75f, 0.5f, 0.5f, 1.f);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw();

  auto& async_loader = AsyncLoader::instance();

  do
  {
    std::this_thread::sleep_for(std::chrono::nanoseconds (10));
    gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw();
  } while (async_loader.is_loading());

  // Clearing alpha from image
  gl.colorMask(false, false, false, true);
  gl.clearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  gl.colorMask(true, true, true, true);

  QPixmap result{};
  result = std::move(QPixmap::fromImage(pixel_buffer.toImage()));
  pixel_buffer.release();

  if (result.isNull())
  {
    throw std::runtime_error("failed rendering " + _filename + " to pixmap");
  }

  return &(_cache[curEntry] = std::move(result));
}


