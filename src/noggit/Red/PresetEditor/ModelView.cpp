#include "ModelView.hpp"
#include <math/projection.hpp>
#include <external/qtimgui/imgui/imgui.h>
#include <external/imguizmo/ImGuizmo.h>

#include <QMatrix4x4>
#include <QVector3D>

using namespace noggit::Red::PresetEditor;

ModelViewer::ModelViewer(QWidget *parent)
: AssetBrowser::ModelViewer(parent, noggit::NoggitRenderContext::PRESET_EDITOR)
, _world(nullptr)
, _world_camera(_camera.position, _camera.yaw(), _camera.pitch())
{
}

void ModelViewer::paintGL()
{
  const qreal now(_startup_time.elapsed() / 1000.0);
  opengl::context::scoped_setter const _ (::gl, context());
  makeCurrent();

  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  tick(now - _last_update);

  _last_update = now;

  MinimapRenderSettings _settings_unused;
  std::map<int, misc::random_color> area_id_colors;

  if (_world)
  {
    _world->draw(world_model_view().transposed()
        , world_projection().transposed()
        , math::vector_3d(0.f, 0.f, 0.f)
        , 0.f
        , math::vector_4d(1.f, 1.f, 1.f, 1.f)
        , CursorType::CIRCLE
        , 0.f
        , false
        , false
        , 0.f
        , math::vector_3d(0.f, 0.f, 0.f)
        , 0.f
        , 0.f
        , false
        , false
        , false
        , false
        , false
        , editing_mode::ground
        , _world_camera.position
        , true
        , false
        , false
        , false
        , true
        , true
        , true
        , true
        , true
        , true
        , false
        , false
        , false
        , &_settings_unused
        , area_id_colors
        , false
        , eTerrainType::eTerrainType_Flat
        , 0
        , display_mode::in_3D

    );
  }

  // Sorting issues may naturally occur here due to draw order. But we can't avoid them if we want world as an underlay.
  // It is just a preview after all.

  draw();

  ImGui::SetCurrentContext(_imgui_context);
  QtImGui::newFrame();


  ImGuizmo::SetDrawlist();


  ImGuizmo::SetOrthographic(false);
  ImGuizmo::BeginFrame();

  ImGuiIO& io = ImGui::GetIO();
  ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

  auto obj_mat = _wmo_instances[0].transform_matrix().transposed();
  float matrixTranslation[3], matrixRotation[3], matrixScale[3];

  ImGuizmo::DecomposeMatrixToComponents(obj_mat, matrixTranslation, matrixRotation, matrixScale);
  ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, obj_mat);

  auto delta_mat = math::matrix_4x4(math::matrix_4x4::unit).transposed();

  auto mview = model_view().transposed();
  auto proj = projection().transposed();

  ImGuizmo::SetID(1);
  ImGuizmo::Manipulate(static_cast<float*>(mview),
                       static_cast<float*>(proj),
                       ImGuizmo::TRANSLATE,
                       ImGuizmo::WORLD,
                       obj_mat,
                       delta_mat,
                       NULL);


  std::string test;
  for (int i = 0; i < 16; ++i)
  {
    test += std::to_string(static_cast<float*>(delta_mat)[i]) + " ";
  }

  QMatrix4x4 new_mat (static_cast<float*>(delta_mat));
  QVector3D pos = {_wmo_instances[0].pos.x, _wmo_instances[0].pos.y, _wmo_instances[0].pos.z};
  QVector3D transform = pos * new_mat;
  _wmo_instances[0].pos = {transform.x(), transform.y(), transform.z()};
  _wmo_instances[0].recalcExtents();

  test += "\n" + std::to_string(_wmo_instances[0].pos.x) + " "
      + std::to_string(_wmo_instances[0].pos.y)
      + " " + std::to_string(_wmo_instances[0].pos.z);



  ImGui::Text(test.c_str());

  ImGui::Render();
}

void ModelViewer::loadWorldUnderlay(const std::string& internal_name, int map_id)
{
  opengl::context::scoped_setter const _ (::gl, context());
  makeCurrent();

  if (map_id < 0)
  {
    _world = nullptr;
  }
  else
  {
    _world = std::make_unique<World> (internal_name, map_id,
                                      noggit::NoggitRenderContext::PRESET_EDITOR);
  }

}

math::matrix_4x4 ModelViewer::world_model_view() const
{
  return _world_camera.look_at_matrix();
}

math::matrix_4x4 ModelViewer::world_projection() const
{
  float far_z = _settings->value("farZ", 2048).toFloat();
  return math::perspective(_world_camera.fov(), aspect_ratio(), 0.1f, far_z);
}

void ModelViewer::tick(float dt)
{
  if (_world)
  {
    _world->mapIndex.enterTile (tile_index (_world_camera.position));
    _world->mapIndex.unloadTiles (tile_index (_world_camera.position));
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

}


