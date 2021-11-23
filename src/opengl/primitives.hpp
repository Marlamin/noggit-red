// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>
#include <noggit/ContextObject.hpp>

#include <memory>
#include <unordered_map>

namespace math
{
  struct vector_3d;
  struct vector_4d;
}

namespace opengl
{
  namespace primitives
  {
    class wire_box
    {
    public:
      wire_box() {}
      wire_box( const wire_box&);
      wire_box& operator=( wire_box& box ) { return *this; };

    public:
      static wire_box& getInstance(noggit::NoggitRenderContext context)
      {
        static std::unordered_map<noggit::NoggitRenderContext, wire_box> instances;

        if (instances.find(context) == instances.end())
        {
          wire_box instance;
          instances[context] = instance;
        }

        return instances.at(context);
      }

      void draw ( glm::mat4x4 const& model_view
                , glm::mat4x4 const& projection
                , glm::mat4x4 const& transform
                , glm::vec4 const& color
                , glm::vec3 const& min_point
                , glm::vec3 const& max_point
                );

      void unload();

    private:
      bool _buffers_are_setup = false;

      void setup_buffers();

      scoped::deferred_upload_vertex_arrays<1> _vao;
      scoped::deferred_upload_buffers<1> _buffers;
      GLuint const& _indices = _buffers[0];
      std::unique_ptr<opengl::program> _program;
    };

    class grid
    {
    public:
        void draw(glm::mat4x4 const& mvp
            , glm::vec3 const& pos
            , glm::vec4  const& color
            , float radius
        );
        void unload();
    private:
        bool _buffers_are_setup = false;

        void setup_buffers();

        int _indice_count = 0;

        scoped::deferred_upload_vertex_arrays<1> _vao;
        scoped::deferred_upload_buffers<2> _buffers;
        GLuint const& _vertices_vbo = _buffers[0];
        GLuint const& _indices_vbo = _buffers[1];
        std::unique_ptr<opengl::program> _program;
    };

    class sphere
    {
    public:
      void draw(glm::mat4x4 const& mvp
               , glm::vec3 const& pos
               , glm::vec4  const& color
               , float radius
               );
      void unload();

    private:
      bool _buffers_are_setup = false;

      void setup_buffers();

      int _indice_count = 0;

      scoped::deferred_upload_vertex_arrays<1> _vao;
      scoped::deferred_upload_buffers<2> _buffers;
      GLuint const& _vertices_vbo = _buffers[0];
      GLuint const& _indices_vbo = _buffers[1];
      std::unique_ptr<opengl::program> _program;
    };

    class square
    {
    public:      
      void draw(glm::mat4x4 const& mvp
               , glm::vec3 const& pos
               , float radius // radius of the biggest circle fitting inside the square drawn
               , math::radians inclination
               , math::radians orientation
               , glm::vec4  const& color
               );
      void unload();
    private:
      bool _buffers_are_setup = false;

      void setup_buffers();

      scoped::deferred_upload_vertex_arrays<1> _vao;
      scoped::deferred_upload_buffers<2> _buffers;
      GLuint const& _vertices_vbo = _buffers[0];
      GLuint const& _indices_vbo = _buffers[1];
      std::unique_ptr<opengl::program> _program;
    };
  }
}
