#include "ModelView.hpp"

using namespace noggit::Red::PresetEditor;

ModelViewer::ModelViewer(QWidget *parent)
: AssetBrowser::ModelViewer(parent, noggit::NoggitRenderContext::PRESET_EDITOR)
, _world(std::make_unique<World> ("azeroth", 0, noggit::NoggitRenderContext::PRESET_EDITOR))
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

  _world->draw(model_view().transposed()
               , projection().transposed()
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
               , math::vector_3d(0.f, 0.f, 0.f)
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

  // Sorting issues may naturally occur here due to draw order. But we can't avoid them if we want world as an underlay.
  // It is just a preview after all.

  draw();
}
