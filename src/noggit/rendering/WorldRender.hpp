// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_WORLDRENDER_HPP
#define NOGGIT_WORLDRENDER_HPP

#include <noggit/rendering/BaseRender.hpp>

#include <external/glm/glm.hpp>
#include <math/trig.hpp>

#include <noggit/tool_enums.hpp>
#include <noggit/rendering/CursorRender.hpp>
#include <noggit/rendering/LiquidTextureManager.hpp>
#include <noggit/map_horizon.h>
#include <noggit/Sky.h>

#include <opengl/shader.hpp>
#include <noggit/rendering/Primitives.hpp>

#include <memory>

class World;
struct MinimapRenderSettings;


struct WorldRenderParams 
{
  float cursorRotation;
  CursorType cursor_type;
  float brush_radius;
  bool show_unpaintable_chunks;
  bool draw_only_inside_light_sphere;
  bool draw_wireframe_light_sphere;
  float alpha_light_sphere;
  float inner_radius_ratio;
  float angle;
  float orientation;
  bool use_ref_pos;
  bool angled_mode;
  bool draw_paintability_overlay;
  editing_mode editing_mode;
  bool camera_moved;
  bool draw_mfbo;
  bool draw_terrain;
  bool draw_wmo;
  bool draw_water;
  bool draw_wmo_doodads;
  bool draw_models;
  bool draw_model_animations;
  bool draw_models_with_box;
  bool draw_hidden_models;
  bool draw_sky;
  bool draw_skybox;
  bool draw_fog;
  eTerrainType ground_editing_brush;
  int water_layer;
  display_mode display_mode;
  bool draw_occlusion_boxes;
  bool minimap_render;
  bool draw_wmo_exterior;

  bool render_select_m2_aabb;
  bool render_select_m2_collission_bbox;
  bool render_select_wmo_aabb;
  bool render_select_wmo_groups_bounds;
};

namespace Noggit::Rendering
{
  class WorldRender : public BaseRender
  {
  public:
    WorldRender(World* world);

    void upload() override;
    void unload() override;

    void draw (glm::mat4x4 const& model_view
        , glm::mat4x4 const& projection
        , glm::vec3 const& cursor_pos
        , glm::vec4 const& cursor_color
        , glm::vec3 const& ref_pos
        , glm::vec3 const& camera_pos
        , MinimapRenderSettings* minimap_render_settings
        , WorldRenderParams const& render_settings
    );

    bool saveMinimap (TileIndex const& tile_idx
                      , MinimapRenderSettings* settings
                      , std::optional<QImage>& combined_image);

    [[nodiscard]]
    OpenGL::TerrainParamsUniformBlock* getTerrainParamsUniformBlock() { return &_terrain_params_ubo_data; };

    void updateTerrainParamsUniformBlock();
    void markTerrainParamsUniformBlockDirty() { _need_terrain_params_ubo_update = true; };

    [[nodiscard]] std::unique_ptr<Skies>& skies() { return _skies; };

    float _view_distance;
    inline float cullDistance() const { return _cull_distance; }

    unsigned int _frame_max_chunk_updates = 256;

    bool directional_lightning;
    bool local_lightning;

  private:

    void drawMinimap ( MapTile *tile
        , glm::mat4x4 const& model_view
        , glm::mat4x4 const& projection
        , glm::vec3 const& camera_pos
        , MinimapRenderSettings* settings
    );

    void updateMVPUniformBlock(const glm::mat4x4& model_view, const glm::mat4x4& projection);
    void updateLightingUniformBlock(bool draw_fog, glm::vec3 const& camera_pos);
    void updateLightingUniformBlockMinimap(MinimapRenderSettings* settings);

    void setupChunkVAO(OpenGL::Scoped::use_program& mcnk_shader);
    void setupLiquidChunkVAO(OpenGL::Scoped::use_program& water_shader);
    void setupOccluderBuffers();
    void setupChunkBuffers();
    void setupLiquidChunkBuffers();

    World* _world;
    float _cull_distance;

    // shaders
    std::unique_ptr<OpenGL::program> _mcnk_program;;
    std::unique_ptr<OpenGL::program> _mfbo_program;
    std::unique_ptr<OpenGL::program> _m2_program;
    std::unique_ptr<OpenGL::program> _m2_instanced_program;
    std::unique_ptr<OpenGL::program> _m2_particles_program;
    std::unique_ptr<OpenGL::program> _m2_ribbons_program;
    std::unique_ptr<OpenGL::program> _m2_box_program;
    std::unique_ptr<OpenGL::program> _wmo_program;
    std::unique_ptr<OpenGL::program> _liquid_program;
    std::unique_ptr<OpenGL::program> _occluder_program;

    // horizon && skies && lighting
    std::unique_ptr<Noggit::map_horizon::render> _horizon_render;
    std::unique_ptr<OutdoorLighting> _outdoor_lighting;
    OutdoorLightStats _outdoor_light_stats;
    std::unique_ptr<Skies> _skies;

    // cursor
    Noggit::CursorRender _cursor_render;
    Noggit::Rendering::Primitives::Sphere _sphere_render;
    Noggit::Rendering::Primitives::Square _square_render;
    Noggit::Rendering::Primitives::Line _line_render;
    Noggit::Rendering::Primitives::WireBox _wirebox_render;

    // buffers
    OpenGL::Scoped::deferred_upload_buffers<8> _buffers;
    GLuint const& _mvp_ubo = _buffers[0];
    GLuint const& _lighting_ubo = _buffers[1];
    GLuint const& _terrain_params_ubo = _buffers[2];
    GLuint const& _mapchunk_vertex = _buffers[3];
    GLuint const& _mapchunk_index = _buffers[4];
    GLuint const& _mapchunk_texcoord = _buffers[5];
    GLuint const& _liquid_chunk_vertex = _buffers[6];
    GLuint const& _occluder_index = _buffers[7];

    // uniform blocks
    OpenGL::MVPUniformBlock _mvp_ubo_data;
    OpenGL::LightingUniformBlock _lighting_ubo_data;
    OpenGL::TerrainParamsUniformBlock _terrain_params_ubo_data;


    // VAOs
    OpenGL::Scoped::deferred_upload_vertex_arrays<3> _vertex_arrays;
    GLuint const& _mapchunk_vao = _vertex_arrays[0];
    GLuint const& _liquid_chunk_vao = _vertex_arrays[1];
    GLuint const& _occluder_vao = _vertex_arrays[2];

    LiquidTextureManager _liquid_texture_manager;

    bool _need_terrain_params_ubo_update = false;
  };
}

#endif //NOGGIT_WORLDRENDER_HPP
