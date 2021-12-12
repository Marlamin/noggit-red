// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <opengl/scoped.hpp>
#include <opengl/shader.fwd.hpp>

#include <memory>

namespace Noggit
{
  class cursor_render
  {
  public:
    enum class mode : int
    {
      circle,
      sphere,
      square,
      cube,
      mode_count
    };

    void draw(mode cursor_mode, glm::mat4x4 const& mvp, glm::vec4 color, glm::vec3 const& pos, float radius, float inner_radius_ratio = 0.f);

      void unload();

  private:
    bool _uploaded = false;

    void upload();

      void create_circle_buffer(OpenGL::Scoped::use_program& shader);
    void create_sphere_buffer(OpenGL::Scoped::use_program& shader);
    void create_square_buffer(OpenGL::Scoped::use_program& shader);
    void create_cube_buffer(OpenGL::Scoped::use_program& shader);

    OpenGL::Scoped::deferred_upload_vertex_arrays<(int)mode::mode_count> _vaos;
    OpenGL::Scoped::deferred_upload_buffers<(int)mode::mode_count * 2> _vbos;

    std::map<mode, int> _indices_count;

    std::unique_ptr<OpenGL::program> _cursor_program;
  };
}
