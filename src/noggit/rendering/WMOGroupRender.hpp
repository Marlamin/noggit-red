// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_WMOGROUPRENDER_HPP
#define NOGGIT_WMOGROUPRENDER_HPP

#include <noggit/rendering/BaseRender.hpp>
#include <opengl/shader.hpp>
#include <opengl/scoped.hpp>
#include <math/frustum.hpp>

class WMOGroup;

namespace Noggit::Rendering
{
  struct WMORenderBatch
  {
    std::uint32_t flags;
    std::uint32_t shader;
    std::uint32_t tex_array0;
    std::uint32_t tex_array1;
    //std::uint32_t tex_array2;
    //std::uint32_t tex_array3;
    std::uint32_t tex0;
    std::uint32_t tex1;
    //std::uint32_t tex2;
    //std::uint32_t tex3;
    std::uint32_t alpha_test_mode;
    std::uint32_t _pad1;
  };

  enum WMORenderBatchFlags
  {
    eWMOBatch_ExteriorLit = 0x1,
    eWMOBatch_HasMOCV = 0x2,
    eWMOBatch_Unlit = 0x4,
    eWMOBatch_Unfogged = 0x8,
    eWMOBatch_Collision = 0x10
  };

  struct WMOCombinedDrawCall
  {
    std::vector<int> samplers;
    std::uint32_t index_start = 0;
    std::uint32_t index_count = 0;
    std::uint32_t n_used_samplers = 0;
    bool backface_cull = false;
  };

  class WMOGroupRender : public BaseRender
  {
  public:
    WMOGroupRender(WMOGroup* wmo_group);

    void upload() override;

    void unload() override;

    void draw( OpenGL::Scoped::use_program& wmo_shader
        , math::frustum const& frustum
        , const float& cull_distance
        , const glm::vec3& camera
        , bool draw_fog
        , bool world_has_skies
    );

    void initRenderBatches();

  private:

    void setupVao(OpenGL::Scoped::use_program& wmo_shader);

    WMOGroup* _wmo_group;

    std::vector<unsigned> _render_batch_mapping;
    std::vector<WMORenderBatch> _render_batches;
    std::vector<WMOCombinedDrawCall> _draw_calls;

    OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vertex_array;
    GLuint const& _vao = _vertex_array[0];
    OpenGL::Scoped::deferred_upload_buffers<8> _buffers;
    GLuint const& _vertices_buffer = _buffers[0];
    GLuint const& _normals_buffer = _buffers[1];
    GLuint const& _texcoords_buffer = _buffers[2];
    GLuint const& _texcoords_buffer_2 = _buffers[3];
    GLuint const& _vertex_colors_buffer = _buffers[4];
    GLuint const& _indices_buffer = _buffers[5];
    GLuint const& _render_batch_mapping_buffer = _buffers[6];
    GLuint const& _render_batch_tex_buffer = _buffers[7];

    GLuint _render_batch_tex;

    bool _uploaded = false;
    bool _vao_is_setup = false;

  };
}

#endif //NOGGIT_WMOGROUPRENDER_HPP
