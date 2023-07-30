// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "WMORender.hpp"
#include <noggit/WMO.h>

using namespace Noggit::Rendering;

WMORender::WMORender(WMO* wmo)
: _wmo(wmo)
{
}

void WMORender::upload()
{

}

void WMORender::unload()
{
  for (auto& group : _wmo->groups)
  {
    group.renderer()->unload();
  }
}

void WMORender::draw(OpenGL::Scoped::use_program& wmo_shader
    , glm::mat4x4 const& model_view
    , glm::mat4x4 const& projection
    , glm::mat4x4 const& transform_matrix
    , bool boundingbox
    , math::frustum const& frustum
    , const float& cull_distance
    , const glm::vec3& camera
    , bool // draw_doodads
    , bool draw_fog
    , int animtime
    , bool world_has_skies
    , display_mode display
    , bool interior_only
)
{

  if (!_wmo->finishedLoading())
  [[unlikely]]
  {
    return;
  }

  wmo_shader.uniform("ambient_color",glm::vec3(_wmo->ambient_light_color));

  for (auto& group : _wmo->groups)
  {
      if (interior_only && !group.is_indoor())
      {
          continue;
      }

    /*
    if (!group.is_visible(transform_matrix, frustum, cull_distance, camera, display))
    {
      continue;
    }

     */

    group.renderer()->draw(wmo_shader
        , frustum
        , cull_distance
        , camera
        , draw_fog
        , world_has_skies
    );

    /*
    group.drawLiquid ( transform_matrix_transposed
                     , render
                     , draw_fog
                     , animtime
                     );

                     */
  }

  if (boundingbox)
  {
    //OpenGL::Scoped::bool_setter<GL_BLEND, GL_TRUE> const blend;
    //gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& group : _wmo->groups)
    {
      Noggit::Rendering::Primitives::WireBox::getInstance(_wmo->_context).draw( model_view
          , projection
          , transform_matrix
          , {1.0f, 1.0f, 1.0f, 1.0f}
          , group.BoundingBoxMin
          , group.BoundingBoxMax
      );
    }

    Noggit::Rendering::Primitives::WireBox::getInstance(_wmo->_context).draw ( model_view
        , projection
        , transform_matrix
        , {1.0f, 0.0f, 0.0f, 1.0f}
        , glm::vec3(_wmo->extents[0].x, _wmo->extents[0].z, -_wmo->extents[0].y)
        , glm::vec3(_wmo->extents[1].x, _wmo->extents[1].z, -_wmo->extents[1].y)
    );

  }
}


bool WMORender::drawSkybox(const glm::mat4x4& model_view, const glm::vec3& camera_pos,
                           OpenGL::Scoped::use_program& m2_shader, const math::frustum& frustum,
                           const float& cull_distance, int animtime, bool draw_particles, glm::vec3 aabb_min,
                           glm::vec3 aabb_max,
                           const std::map<int, std::pair<glm::vec3, glm::vec3>>& group_extents) const
{
  if (!_wmo->skybox || !math::is_inside_of(camera_pos,aabb_min, aabb_max))
  {
    return false;
  }

  for (int i=0; i < _wmo->groups.size(); ++i)
  {
    auto const& g = _wmo->groups[i];

    if (!g.has_skybox())
    {
      continue;
    }

    auto& extent(group_extents.at(i));

    if (math::is_inside_of(camera_pos, extent.first, extent.second))
    {
      ModelInstance sky(_wmo->skybox.value()->file_key().filepath(), _wmo->_context);
      sky.pos = camera_pos;
      sky.scale = 2.f;
      sky.recalcExtents();

      OpenGL::M2RenderState model_render_state;
      model_render_state.tex_arrays = {0, 0};
      model_render_state.tex_indices = {0, 0};
      model_render_state.tex_unit_lookups = {-1, -1};
      gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      gl.disable(GL_BLEND);
      gl.depthMask(GL_TRUE);
      m2_shader.uniform("blend_mode", 0);
      m2_shader.uniform("unfogged", static_cast<int>(model_render_state.unfogged));
      m2_shader.uniform("unlit",  static_cast<int>(model_render_state.unlit));
      m2_shader.uniform("tex_unit_lookup_1", 0);
      m2_shader.uniform("tex_unit_lookup_2", 0);
      m2_shader.uniform("pixel_shader", 0);

      _wmo->skybox->get()->renderer()->draw(model_view, sky, m2_shader, model_render_state, frustum, cull_distance, camera_pos, animtime, display_mode::in_3D);

      return true;
    }
  }

  return false;
}
