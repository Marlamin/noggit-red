// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_WMORENDER_HPP
#define NOGGIT_WMORENDER_HPP

#include <noggit/rendering/BaseRender.hpp>
#include <noggit/tool_enums.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>
#include <glm/glm.hpp>
#include <math/frustum.hpp>

#include <map>

class WMO;

namespace Noggit::Rendering
{
  class WMORender : public BaseRender
  {
  public:
    WMORender(WMO* wmo);

    void upload() override;
    void unload() override;

    void draw(OpenGL::Scoped::use_program& wmo_shader
        , glm::mat4x4 const& model_view
        , glm::mat4x4 const& projection
        , glm::mat4x4 const& transform_matrix
        , bool boundingbox
        , math::frustum const& frustum
        , const float& cull_distance
        , const glm::vec3& camera
        , bool draw_doodads
        , bool draw_fog
        , int animtime
        , bool world_has_skies
        , display_mode display
    );

    bool drawSkybox(glm::mat4x4 const& model_view
        , glm::vec3 const& camera_pos
        , OpenGL::Scoped::use_program& m2_shader
        , math::frustum const& frustum
        , const float& cull_distance
        , int animtime
        , bool draw_particles
        , glm::vec3 aabb_min
        , glm::vec3 aabb_max
        , std::map<int, std::pair<glm::vec3, glm::vec3>> const& group_extents
    ) const;

  private:
    WMO* _wmo;
  };
}

#endif //NOGGIT_WMORENDER_HPP
