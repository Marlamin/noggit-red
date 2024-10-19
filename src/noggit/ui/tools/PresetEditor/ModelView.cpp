#include "ModelView.hpp"
#include <external/qtimgui/imgui/imgui.h>
#include <external/imguizmo/ImGuizmo.h>

#include <vector>

using namespace Noggit::Ui::Tools::PresetEditor;

ModelViewer::ModelViewer(QWidget *parent)
: AssetBrowser::ModelViewer(parent, Noggit::NoggitRenderContext::PRESET_EDITOR)
, _world(nullptr)
, _world_camera(_camera.position, _camera.yaw(), _camera.pitch())
, _transform_gizmo(Noggit::Ui::Tools::ViewportGizmo::GizmoContext::PRESET_EDITOR)
{
}

void ModelViewer::paintGL()
{
  const qreal now(_startup_time.elapsed() / 1000.0);
  OpenGL::context::scoped_setter const _ (::gl, context());
  makeCurrent();

  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  tick(now - _last_update);

  _last_update = now;

  MinimapRenderSettings _settings_unused;

  WorldRenderParams renderParams;

  renderParams.cursorRotation = 0.0f;
  renderParams.cursor_type = CursorType::CIRCLE;
  renderParams.brush_radius = 0.f;
  renderParams.show_unpaintable_chunks = false;
  renderParams.draw_only_inside_light_sphere = false;
  renderParams.draw_wireframe_light_sphere = false;
  renderParams.alpha_light_sphere = 0.3f;
  renderParams.inner_radius_ratio = 0.0f;
  renderParams.angle = 0.0f;
  renderParams.orientation = 0.0f;
  renderParams.use_ref_pos = false;
  renderParams.angled_mode = false;
  renderParams.draw_paintability_overlay = false;
  renderParams.editing_mode = editing_mode::ground;
  renderParams.camera_moved = true;
  renderParams.draw_mfbo = false;
  renderParams.draw_terrain = true;
  renderParams.draw_wmo = true;
  renderParams.draw_water = true;
  renderParams.draw_wmo_doodads = true;
  renderParams.draw_models = true;
  renderParams.draw_model_animations = true;
  renderParams.draw_models_with_box = false;
  renderParams.draw_hidden_models = true;
  renderParams.draw_sky = false;
  renderParams.draw_skybox = false;
  renderParams.draw_fog = false;
  renderParams.ground_editing_brush = eTerrainType::eTerrainType_Flat;
  renderParams.water_layer = 0;
  renderParams.display_mode = display_mode::in_3D;
  renderParams.draw_occlusion_boxes = false;
  renderParams.minimap_render = false;
  renderParams.draw_wmo_exterior = true;
  renderParams.render_select_m2_aabb = false;
  renderParams.render_select_m2_collission_bbox = false;
  renderParams.render_select_wmo_aabb = false;
  renderParams.render_select_wmo_groups_bounds = false;


  if (_world)
  {
    _world->renderer()->draw(world_model_view()
        , world_projection()
        , glm::vec3(0.f, 0.f, 0.f)
        , glm::vec4(1.f, 1.f, 1.f, 1.f)
        , glm::vec3(0.f, 0.f, 0.f)
        , _world_camera.position
        , &_settings_unused
        , renderParams

    );
  }

  // Sorting issues may naturally occur here due to draw order. But we can't avoid them if we want world as an underlay.
  // It is just a preview after all.

  draw();

  if (_gizmo_on.get())
  {
    ImGui::SetCurrentContext(_imgui_context);
    QtImGui::newFrame();

    static bool is_open = false;
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowPos(ImVec2(-100.f, -100.f));
    ImGui::Begin("Gizmo", &is_open, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar
                                        | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground);

    auto mv = model_view();
    auto proj = projection();

    _transform_gizmo.setCurrentGizmoOperation(_gizmo_operation);
    _transform_gizmo.setCurrentGizmoMode(_gizmo_mode);
    _transform_gizmo.setUseMultiselectionPivot(false);

    //_transform_gizmo.setMultiselectionPivot(pivot);
    // TEMP HACK!!!
    std::vector<selection_type> selection;
    selection.emplace_back(&_wmo_instances[0]);

    _transform_gizmo.handleTransformGizmo(nullptr, selection, mv, proj);

    //_world->update_selection_pivot();

    ImGui::End();
    ImGui::Render();

  }
}

void ModelViewer::loadWorldUnderlay(const std::string& internal_name, int map_id)
{
  OpenGL::context::scoped_setter const _ (::gl, context());
  makeCurrent();

  if (map_id < 0)
  {
    _world = nullptr;
  }
  else
  {
    _world = std::make_unique<World> (internal_name, map_id,
                                      Noggit::NoggitRenderContext::PRESET_EDITOR);
  }

}

glm::mat4x4 ModelViewer::world_model_view() const
{
  return _world_camera.look_at_matrix();
}

glm::mat4x4 ModelViewer::world_projection() const
{
  float far_z = _settings->value("view_distance", 2000.f).toFloat();
  return glm::perspective(_camera.fov()._, aspect_ratio(), 1.f, far_z);
}

void ModelViewer::tick(float dt)
{
  if (_world)
  {
    _world->mapIndex.enterTile (TileIndex (_world_camera.position));
    _world->mapIndex.unloadTiles (TileIndex (_world_camera.position));
  }

  AssetBrowser::ModelViewer::tick(dt);

  if (turn)
  {
    _world_camera.add_to_yaw(math::degrees(turn));
  }
  if (lookat)
  {
    _world_camera.add_to_pitch(math::degrees(lookat));
  }
  if (moving)
  {
    _world_camera.move_forward(moving, dt);
  }
  if (strafing)
  {
    _world_camera.move_horizontal(strafing, dt);
  }
  if (updown)
  {
    _world_camera.move_vertical(updown, dt);
  }

}

void ModelViewer::mouseMoveEvent(QMouseEvent *event)
{

  QLineF const relative_movement (_last_mouse_pos, event->pos());

  if (look)
  {
    _world_camera.add_to_yaw(math::degrees(relative_movement.dx() / 20.0f));
    _world_camera.add_to_pitch(math::degrees(mousedir * relative_movement.dy() / 20.0f));
  }

  AssetBrowser::ModelViewer::mouseMoveEvent(event);
}

void ModelViewer::initializeGL()
{
  AssetBrowser::ModelViewer::initializeGL();
  _imgui_context = QtImGui::initialize(this);

  connect(context(), &QOpenGLContext::aboutToBeDestroyed, [=]()
  {
    emit aboutToLooseContext();
  });

}


