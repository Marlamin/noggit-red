#include "PreviewRenderer.hpp"

#include <opengl/scoped.hpp>
#include <opengl/primitives.hpp>
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
#include <QColor>
#include <QMatrix4x4>
#include <QVector3D>


using namespace noggit::ui::tools;


PreviewRenderer::PreviewRenderer(int width, int height, noggit::NoggitRenderContext context, QWidget* parent)
  :  noggit::ui::tools::ViewportManager::Viewport(parent)
  , _camera (glm::vec3(0.0f, 0.0f, 0.0f), math::degrees(0.0f), math::degrees(0.0f))
  , _settings (new QSettings())
  , _width(width)
  , _height(height)
  , _liquid_texture_manager(context)
{
  _context = context;
  _cache = {};

  opengl::context::save_current_context const context_save (::gl);
  _offscreen_context.create();

  _fmt.setSamples(1);
  _fmt.setInternalTextureFormat(GL_RGBA8);
  _fmt.setAttachment(QOpenGLFramebufferObject::Depth);

  _offscreen_surface.create();
  _offscreen_context.makeCurrent(&_offscreen_surface);

  opengl::context::scoped_setter const context_set (::gl, &_offscreen_context);

  _light_dir = glm::vec3(0.0f, 1.0f, 0.0f);
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

  _lighting_needs_update = true;

  auto diffuse_color = _settings->value("assetBrowser/diffuse_light",
    QVariant::fromValue(QColor::fromRgbF(1.0f, 0.532352924f, 0.0f))).value<QColor>();
  _diffuse_light = {static_cast<float>(diffuse_color.redF()),
                    static_cast<float>(diffuse_color.greenF()),
                    static_cast<float>(diffuse_color.blueF())};

 auto ambient_color = _settings->value("assetBrowser/ambient_light",
     QVariant::fromValue(QColor::fromRgbF(0.407770514f, 0.508424163f, 0.602650642f))).value<QColor>();

 _ambient_light = {static_cast<float>(ambient_color.redF()),
                   static_cast<float>(ambient_color.greenF()),
                   static_cast<float>(ambient_color.blueF())};

  auto background_color = _settings->value("assetBrowser/background_color",
     QVariant::fromValue(QColor(127, 127, 127))).value<QColor>();

  _background_color = {static_cast<float>(background_color.redF()),
                       static_cast<float>(background_color.greenF()),
                       static_cast<float>(background_color.blueF())};

  resetCamera();
}

void PreviewRenderer::setModelOffscreen(std::string const& filename)
{
  opengl::context::save_current_context const context_save (::gl);
  _offscreen_context.makeCurrent(&_offscreen_surface);
  opengl::context::scoped_setter const context_set (::gl, &_offscreen_context);

  setModel(filename);
}


void PreviewRenderer::resetCamera(float x, float y, float z, float roll, float yaw, float pitch)
{
  _camera.reset(x, y, z, roll, yaw, pitch);
  float radius = 0.f;

  std::vector<glm::vec3> extents = calcSceneExtents();
  _camera.position = (extents[0] + extents[1]) / 2.0f;
  radius = std::max(glm::distance(_camera.position, extents[0]), glm::distance(_camera.position, extents[1]));

  float distance_factor = abs( radius / sin(_camera.fov()._ / 3.f));
  _camera.move_forward_factor(-1.f, distance_factor);

}


void PreviewRenderer::draw()
{
  if (!_uploaded)
  [[unlikely]]
  {  
    upload();
  }

  gl.clearColor(_background_color.r, _background_color.g, _background_color.b, 1.0f);
  gl.depthMask(GL_TRUE);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float culldistance = 10000000;

  auto mv = model_view();
  auto proj = projection();

  glm::mat4x4 const mvp(proj * mv);
  math::frustum const frustum (glm::transpose(mvp));

  updateMVPUniformBlock(mv, proj);

  if (_lighting_needs_update)
    updateLightingUniformBlock();

  gl.enable(GL_DEPTH_TEST);
  gl.depthFunc(GL_LEQUAL);
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // draw WMOs
  std::unordered_map<std::string, std::vector<ModelInstance*>> _wmo_doodads;

  if (_draw_wmo.get() && !_wmo_instances.empty())
  {
    /* set anim time only once per frame
    {
      opengl::scoped::use_program water_shader {_liquid_render->shader_program()};
      water_shader.uniform("animtime", _animtime / 2880.f);

      //water_shader.uniform("model_view", model_view().transposed());
      //water_shader.uniform("projection", projection().transposed());

      //water_shader.uniform("ocean_color_light", ocean_color_light);
      //water_shader.uniform("ocean_color_dark", ocean_color_dark);
      //water_shader.uniform("river_color_light", river_color_light);
      //water_shader.uniform("river_color_dark", river_color_dark);
      //water_shader.uniform("use_transform", 1);
    }

     */

    {
      opengl::scoped::use_program wmo_program{*_wmo_program.get()};

      wmo_program.uniform("camera", glm::vec3(_camera.position.x, _camera.position.y, _camera.position.z));


      for (auto& wmo_instance : _wmo_instances)
      {
        wmo_instance.wmo->wait_until_loaded();
        wmo_instance.wmo->waitForChildrenLoaded();
        wmo_instance.ensureExtents();
        wmo_instance.draw(
            wmo_program, model_view(), projection(), frustum, culldistance,
            _camera.position, _draw_boxes.get(), _draw_models.get() 
            , false, std::vector<selection_type>(), 0, false, display_mode::in_3D, true
        );

        for (auto& pair : *wmo_instance.get_doodads(true))
        {
          for (auto& doodad : pair.second)
            _wmo_doodads[doodad.model->_file_key.filepath()].push_back(&doodad);
        }
     }

    }

  }

  // draw M2
  std::unordered_map<Model*, std::size_t> model_boxes_to_draw;

  if (_draw_models.get() && !(_model_instances.empty() && _wmo_doodads.empty()))
  {
    if (_draw_animated.get())
      ModelManager::resetAnim();

    opengl::scoped::use_program m2_shader {*_m2_instanced_program.get()};

    opengl::M2RenderState model_render_state;
    model_render_state.tex_arrays = { 0, 0 };
    model_render_state.tex_indices = { 0, 0 };
    model_render_state.tex_unit_lookups = { 0, 0 };
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.disable(GL_BLEND);
    gl.depthMask(GL_TRUE);
    m2_shader.uniform("blend_mode", 0);
    m2_shader.uniform("unfogged", static_cast<int>(model_render_state.unfogged));
    m2_shader.uniform("unlit", static_cast<int>(model_render_state.unlit));
    m2_shader.uniform("tex_unit_lookup_1", 0);
    m2_shader.uniform("tex_unit_lookup_2", 0);
    m2_shader.uniform("pixel_shader", 0);

    std::vector<ModelInstance*> instance{ nullptr };
    std::vector<glm::mat4x4> instance_mtx{ glm::mat4x4(1)};

    for (auto& model_instance : _model_instances)
    {
      model_instance.model->wait_until_loaded();
      model_instance.model->waitForChildrenLoaded();
      instance[0] = &model_instance;
      instance_mtx[0] = model_instance.transformMatrix();

      model_instance.model->draw(
        mv
        , instance_mtx
        , m2_shader
        , model_render_state
        , frustum
        , culldistance
        , _camera.position
        , _animtime
        , _draw_boxes.get()
        , model_boxes_to_draw
        , display_mode::in_3D
      );
    }

    for (auto& it : _wmo_doodads)
    {
      instance_mtx.clear();
      
      for (auto& instance : it.second)
      {
        instance_mtx.push_back(instance->transformMatrix());
      }

      it.second[0]->model->draw(
          mv
          , instance_mtx
          , m2_shader
          , model_render_state
          , frustum
          , culldistance
          , _camera.position
          , _animtime
          , _draw_boxes.get()
          , model_boxes_to_draw
          , display_mode::in_3D
      );
    }


    if(_draw_boxes.get() && !model_boxes_to_draw.empty())
    {
      opengl::scoped::use_program m2_box_shader{ *_m2_box_program.get() };

      opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);

      for (auto& it : model_boxes_to_draw)
      {
        glm::vec4 color = it.first->is_hidden()
                                ? glm::vec4(0.f, 0.f, 1.f, 1.f)
                                : ( it.first->use_fake_geometry()
                                    ? glm::vec4(1.f, 0.f, 0.f, 1.f)
                                    : glm::vec4(0.75f, 0.75f, 0.75f, 1.f)
                                )
        ;

        m2_box_shader.uniform("color", color);
        it.first->draw_box(m2_box_shader, it.second);
      }
    }
  }


  gl.bindVertexArray(0);
  gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // model particles

  /*
  if (_draw_animated.get() && !model_with_particles.empty())
  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    opengl::scoped::use_program particles_shader {*_m2_particles_program.get()};

    particles_shader.uniform("model_view_projection", mvp);
    particles_shader.uniform("tex", 0);
    opengl::texture::set_active_texture(0);

    for (auto& it : model_with_particles)
    {
      it.first->draw_particles(model_view().transposed(), particles_shader, it.second);
    }
  }

  if (_draw_animated.get() && !model_with_particles.empty())
  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    opengl::scoped::use_program ribbon_shader {*_m2_ribbons_program.get()};

    ribbon_shader.uniform("model_view_projection", mvp);
    ribbon_shader.uniform("tex", 0);

    gl.blendFunc(GL_SRC_ALPHA, GL_ONE);

    for (auto& it : model_with_particles)
    {
      it.first->draw_ribbons(ribbon_shader, it.second);
    }
  }

  */

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (_draw_grid.get())
  {
    _grid.draw(mvp, glm::vec3(0.f, 0.f, 0.f),
               glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), 30.f);

  }

}

glm::mat4x4 PreviewRenderer::model_view() const
{
  return _camera.look_at_matrix();
}

glm::mat4x4 PreviewRenderer::projection() const
{
  float far_z = _settings->value("farZ", 2048).toFloat();
  return glm::perspective(_camera.fov()._, aspect_ratio(), 1.f, far_z);
}

float PreviewRenderer::aspect_ratio() const
{
  return static_cast<float>(_width) / static_cast<float>(_height);
}

std::vector<glm::vec3> PreviewRenderer::calcSceneExtents()
{
  glm::vec3 min = {std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max()};

  glm::vec3 max = {std::numeric_limits<float>::min(),
                         std::numeric_limits<float>::min(),
                         std::numeric_limits<float>::min()};

  for (auto& instance : _model_instances)
  {
    for (int i = 0; i < 3; ++i)
    {
      min[i] = std::min(instance.extents[0][i], min[i]);
      max[i] = std::max(instance.extents[1][i], max[i]);
    }
  }

  for (auto& instance : _wmo_instances)
  {
    for (int i = 0; i < 3; ++i)
    {
      min[i] = std::min(instance.extents[0][i], min[i]);
      max[i] = std::max(instance.extents[1][i], max[i]);
    }
  }

  return std::move(std::vector<glm::vec3>{min, max});
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
  gl.clearColor(_background_color.r, _background_color.g, _background_color.b, 1.f);
  gl.depthMask(GL_TRUE);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  tick(1.0f);
  draw();

  auto& async_loader = AsyncLoader::instance();

  if (async_loader.is_loading())
  {
    // wait for the loader to finish
    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while (async_loader.is_loading());

    // redraw
    gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw();
  }

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

void PreviewRenderer::setLightDirection(float y, float z)
{
  _light_dir = {1.f, 0.5f, 0.f};
  QMatrix4x4 matrix = QMatrix4x4();
  matrix.rotate(z, 0.f, 1.f, 0.f);
  matrix.rotate(y, 1.f, 0.f, 0.f);

  QVector3D light_dir = {_light_dir.x, _light_dir.y, _light_dir.z};
  light_dir = matrix * light_dir;

  _light_dir.x = light_dir.x();
  _light_dir.y = light_dir.y();
  _light_dir.z = light_dir.z();

  _lighting_needs_update = true;
}


void PreviewRenderer::update_emitters(float dt)
{
  while (dt > 0.1f)
  {
    ModelManager::updateEmitters(0.1f);
    dt -= 0.1f;
  }
  ModelManager::updateEmitters(dt);
}

void PreviewRenderer::tick(float dt)
{
  dt = std::min(dt, 1.0f);

  _animtime += dt * 1000.0f;

  if (_draw_animated.get())
  {
    update_emitters(dt);
  }
}

PreviewRenderer::~PreviewRenderer()
{
  _destroying = true;

  if (_offscreen_mode)
  {
    opengl::context::save_current_context const context_save (::gl);
    _offscreen_context.makeCurrent(&_offscreen_surface);
    opengl::context::scoped_setter const context_set (::gl, &_offscreen_context);

    _model_instances.clear();
    _wmo_instances.clear();

    _m2_program.reset();
    _m2_instanced_program.reset();
    _m2_particles_program.reset();
    _m2_ribbons_program.reset();
    _m2_box_program.reset();
    _wmo_program.reset();

  }
  else
  {
    makeCurrent();
    opengl::context::scoped_setter const context_set (::gl, context());

    _model_instances.clear();
    _wmo_instances.clear();

    _m2_program.reset();
    _m2_instanced_program.reset();
    _m2_particles_program.reset();
    _m2_ribbons_program.reset();
    _m2_box_program.reset();
    _wmo_program.reset();
  }

}

void PreviewRenderer::upload()
{
  _buffers.upload();

  // m2

  _m2_program.reset
  (new opengl::program
    { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs") }
      , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
    }
  );

  {
    opengl::scoped::use_program m2_shader{ *_m2_program.get() };
    m2_shader.uniform("bone_matrices", 0);
    m2_shader.uniform("tex1", 1);
    m2_shader.uniform("tex2", 2);

    m2_shader.bind_uniform_block("matrices", 0);
    gl.bindBuffer(GL_UNIFORM_BUFFER, _mvp_ubo);
    gl.bufferData(GL_UNIFORM_BUFFER, sizeof(opengl::MVPUniformBlock), NULL, GL_DYNAMIC_DRAW);
    gl.bindBufferRange(GL_UNIFORM_BUFFER, opengl::ubo_targets::MVP, _mvp_ubo, 0, sizeof(opengl::MVPUniformBlock));
    gl.bindBuffer(GL_UNIFORM_BUFFER, 0);

    m2_shader.bind_uniform_block("lighting", 1);
    gl.bindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
    gl.bufferData(GL_UNIFORM_BUFFER, sizeof(opengl::LightingUniformBlock), NULL, GL_DYNAMIC_DRAW);
    gl.bindBufferRange(GL_UNIFORM_BUFFER, opengl::ubo_targets::LIGHTING, _lighting_ubo, 0, sizeof(opengl::LightingUniformBlock));
    gl.bindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  // m2 instaced
  
  _m2_instanced_program.reset
  (new opengl::program
    { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs", {"instanced"}) }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
    }
  );

  {
    opengl::scoped::use_program m2_shader_instanced{ *_m2_instanced_program.get() };
    m2_shader_instanced.bind_uniform_block("matrices", 0);
    m2_shader_instanced.bind_uniform_block("lighting", 1);
    m2_shader_instanced.uniform("bone_matrices", 0);
    m2_shader_instanced.uniform("tex1", 1);
    m2_shader_instanced.uniform("tex2", 2);
  }
 
  // m2 box

  _m2_box_program.reset
  (new opengl::program
    { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_box_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_box_fs") }
    }
  );

  {
    opengl::scoped::use_program m2_box_shader{ *_m2_box_program.get() };
    m2_box_shader.bind_uniform_block("matrices", 0);
  }


  /*
  

  _m2_ribbons_program.reset
  (new opengl::program
    { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("ribbon_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("ribbon_fs") }
    }
  );
  

  _m2_particles_program.reset
  (new opengl::program
    { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("particle_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("particle_fs") }
    }
  );

  */

  // wmo
  
  _wmo_program.reset
  (new opengl::program
    { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("wmo_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("wmo_fs") }
    }
  );

  {
    std::vector<int> samplers{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    opengl::scoped::use_program wmo_program{ *_wmo_program.get() };
    wmo_program.uniform("render_batches_tex", 0);
    wmo_program.uniform("texture_samplers", samplers);
    wmo_program.bind_uniform_block("matrices", 0);
    wmo_program.bind_uniform_block("lighting", 1);
  }

  // liquid
  _liquid_texture_manager.upload();
  _liquid_program.reset
    (new opengl::program
      { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("liquid_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("liquid_fs") }
      }
    );

  {
    opengl::scoped::use_program liquid_render{ *_liquid_program.get() };

    //setupLiquidChunkBuffers();
    //setupLiquidChunkVAO(liquid_render);

    static std::vector<int> samplers{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    liquid_render.bind_uniform_block("matrices", 0);
    liquid_render.bind_uniform_block("lighting", 1);
    liquid_render.bind_uniform_block("liquid_layers_params", 4);
    liquid_render.uniform("vertex_data", 0);
    liquid_render.uniform("texture_samplers", samplers);

  }

  setModel("world/wmo/azeroth/buildings/human_farm/farm.wmo");

  auto background_color = _settings->value("assetBrowser/background_color",
    QVariant::fromValue(QColor(127, 127, 127))).value<QColor>();

  _background_color = { static_cast<float>(background_color.redF()),
                       static_cast<float>(background_color.greenF()),
                       static_cast<float>(background_color.blueF()) };
 
  _uploaded = true;

}


void PreviewRenderer::unload()
{
  _buffers.unload();

  _m2_program.reset();
  _m2_instanced_program.reset();
  _m2_particles_program.reset();
  _m2_ribbons_program.reset();
  _m2_box_program.reset();
  _wmo_program.reset();
  _liquid_program.reset();
  _liquid_texture_manager.unload();

  _uploaded = false;

}

void PreviewRenderer::unloadOpenglData(bool from_manager)
{
  if (_offscreen_mode)
    return;

  LogDebug << "Changing context of Asset Browser / Preset Editor." << std::endl;
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());

  ModelManager::unload_all(_context);
  WMOManager::unload_all(_context);
  TextureManager::unload_all(_context);

  unload();

  LogDebug << "Changed context of Asset Browser / Preset Editor.." << std::endl;

  if (!from_manager)
    ViewportManager::ViewportManager::unloadOpenglData(this);
}

void noggit::ui::tools::PreviewRenderer::updateLightingUniformBlock()
{

  glm::vec4 ocean_color_light(glm::vec3(1.0f, 1.0f, 1.0f), 1.f);
  glm::vec4 ocean_color_dark(glm::vec3(1.0f, 1.0f, 1.0f), 1.f);
  glm::vec4 river_color_light(glm::vec3(1.0f, 1.0f, 1.0f), 1.f);
  glm::vec4 river_color_dark(glm::vec3(1.0f, 1.0f, 1.0f), 1.f);

  _lighting_ubo_data.DiffuseColor_FogStart = { _diffuse_light.x,_diffuse_light.y,_diffuse_light.z, 0};
  _lighting_ubo_data.AmbientColor_FogEnd = { _ambient_light.x, _ambient_light.y, _ambient_light.z, 0};
  _lighting_ubo_data.FogColor_FogOn = { 0, 0, 0, 0};
  _lighting_ubo_data.LightDir_FogRate = { _light_dir.x, _light_dir.y, _light_dir.z, 1.0f};
  _lighting_ubo_data.OceanColorLight = { 1.0f, 1.0f, 1.0f, 1.0f };
  _lighting_ubo_data.OceanColorDark = { 1.0f, 1.0f, 1.0f, 1.0f };
  _lighting_ubo_data.RiverColorLight = { 1.0f, 1.0f, 1.0f, 1.0f };
  _lighting_ubo_data.RiverColorDark = { 1.0f, 1.0f, 1.0f, 1.0f };

  gl.bindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
  gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(opengl::LightingUniformBlock), &_lighting_ubo_data);
  
  _lighting_needs_update = false;
}

void noggit::ui::tools::PreviewRenderer::updateMVPUniformBlock(const glm::mat4x4& model_view, const glm::mat4x4& projection)
{
  _mvp_ubo_data.model_view = model_view;
  _mvp_ubo_data.projection = projection;

  gl.bindBuffer(GL_UNIFORM_BUFFER, _mvp_ubo);
  gl.bufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(opengl::MVPUniformBlock), &_mvp_ubo_data);

}
