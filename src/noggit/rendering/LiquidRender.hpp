// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_LIQUIDRENDER_HPP
#define NOGGIT_LIQUIDRENDER_HPP

#include <noggit/rendering/BaseRender.hpp>
#include <noggit/tool_enums.hpp>

#include <opengl/types.hpp>

#include <glm/glm.hpp>

namespace math
{
  class frustum;
}

namespace OpenGL::Scoped
{
  struct use_program;
}

class MapTile;


namespace Noggit::Rendering
{
  class LiquidTextureManager;

  struct LiquidLayerDrawCallData
  {
    unsigned n_used_chunks = 0;
    std::array<OpenGL::LiquidChunkInstanceDataUniformBlock, 256> chunk_data;
    std::array<std::array<glm::vec4, 9 * 9>, 256> vertex_data ;
    std::vector<int> texture_samplers;
    GLuint chunk_data_buf = 0;
    GLuint vertex_data_tex = 0;
  };


  class LiquidRender : public BaseRender
  {
  public:
    explicit LiquidRender(MapTile* map_tile);

    void upload() override;
    void unload() override;

    void draw(
        math::frustum const& frustum
        , const glm::vec3& camera
        , bool camera_moved
        , OpenGL::Scoped::use_program& water_shader
        , int animtime
        , int layer
        , display_mode display
        , LiquidTextureManager* tex_manager
    );

    bool needsUpdate() const { return _need_buffer_update; };
    void tagUpdate() { _need_buffer_update = true; }

  private:
    void updateLayerData(LiquidTextureManager* tex_manager);

    MapTile* _map_tile;

    std::vector<LiquidLayerDrawCallData> _render_layers;

    bool _need_buffer_update = false;

  };
}

#endif //NOGGIT_LIQUIDRENDER_HPP
