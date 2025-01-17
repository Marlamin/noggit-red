// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_TILERENDER_HPP
#define NOGGIT_TILERENDER_HPP

#include <noggit/rendering/BaseRender.hpp>
#include <opengl/scoped.hpp>
#include <array>

namespace OpenGL::Scoped
{
  struct use_program;
}

class MapTile;
class MapChunk;

namespace Noggit::Rendering
{
  struct MapTileDrawCall
  {
    std::array<int, 11> samplers;
    unsigned start_chunk;
    unsigned n_chunks;
  };

  class TileRender : public BaseRender
  {
  public:
    explicit TileRender(MapTile* map_tile);

    void upload() override;
    void unload() override;

    void draw(OpenGL::Scoped::use_program& mcnk_shader
      , const glm::vec3& camera
      , bool show_unpaintable_chunks
      , bool draw_paintability_overlay
      , bool is_selected
      , bool skip_upload_alphamap = false
    );

    void doTileOcclusionQuery(OpenGL::Scoped::use_program& occlusion_shader);
    bool getTileOcclusionQueryResult(glm::vec3 const& camera);
    void discardTileOcclusionQuery();
    void notifyTileRendererOnSelectedTextureChange();;
    void setChunkGroundEffectColor(unsigned int chunkid, glm::vec3 color);

    void initChunkData(MapChunk* chunk);

    void setChunkDetaildoodadsExclusionData(MapChunk* chunk);
    void setChunkGroundEffectActiveData(MapChunk* chunk);
    void setActiveRenderGEffectTexture(std::string active_texture);

    [[nodiscard]]
    unsigned objectsFrustumCullTest() const;;
    void setObjectsFrustumCullTest(unsigned state);;

    [[nodiscard]]
    bool isOccluded() const; ;
    void setOccluded(bool state);;

    [[nodiscard]]
    bool isFrustumCulled() const;;
    void setFrustumCulled(bool state);;

    [[nodiscard]]
    bool isOverridingOcclusionCulling() const;;
    void setOverrideOcclusionCulling(bool state);;

    [[nodiscard]]
    bool isUploaded() const;;
    [[nodiscard]]
    bool alphamapUploadedLastFrame() const;;
    int numUploadedChunkAlphamaps() const;;

  private:

    void uploadTextures();
    bool fillSamplers(MapChunk* chunk, unsigned chunk_index, unsigned draw_call_index);

    MapTile* _map_tile;

    bool _uploaded = false;
    bool _selected = false;
    bool _split_drawcall = false;
    bool _requires_sampler_reset = false;
    bool _requires_paintability_recalc = true;
    bool _requires_ground_effect_color_recalc = true;
    bool _texture_not_loaded = true;
    bool _require_geffect_active_texture_update = true;

    bool _uploaded_alphamap_last_frame = false;
    int _num_uploaded_chunk_alphamaps = 0; // last frame

    std::string _geffect_active_texture = "";

    // culling
    unsigned _objects_frustum_cull_test = 0;
    bool _tile_occluded = false;
    bool _tile_frustum_culled = true;
    bool _tile_occlusion_cull_override = true;

    // drawing
    std::vector<MapTileDrawCall> _draw_calls;

    OpenGL::Scoped::deferred_upload_textures<4> _chunk_texture_arrays;
    GLuint const& _height_tex = _chunk_texture_arrays[0];
    GLuint const& _mccv_tex = _chunk_texture_arrays[1];
    GLuint const& _shadowmap_tex = _chunk_texture_arrays[2];
    GLuint const& _alphamap_tex = _chunk_texture_arrays[3];

    GLuint _tile_occlusion_query;
    bool _tile_occlusion_query_in_use = false;

    OpenGL::Scoped::deferred_upload_buffers<1> _buffers;

    GLuint const& _chunk_instance_data_ubo = _buffers[0];
    OpenGL::ChunkInstanceDataUniformBlock _chunk_instance_data[256];

  };
}

#endif //NOGGIT_TILERENDER_HPP
