// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>
#include <math/trig.hpp>
#include <noggit/ContextObject.hpp>

#include <memory>
#include <unordered_map>

namespace math
{
  struct vector_3d;
  struct vector_4d;
}

class World;
namespace Noggit::Rendering::Primitives
{
  class WireBox
  {
  public:
    WireBox() {}
    WireBox(const WireBox&);
    WireBox& operator=(WireBox& box ) { return *this; };

  public:
    static WireBox& getInstance(Noggit::NoggitRenderContext context)
    {
      static std::unordered_map<Noggit::NoggitRenderContext, WireBox> instances;

      if (instances.find(context) == instances.end())
      {
        WireBox instance;
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

    OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
    OpenGL::Scoped::deferred_upload_buffers<1> _buffers;
    GLuint const& _indices = _buffers[0];
    std::unique_ptr<OpenGL::program> _program;
  };

  class Grid
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

      OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
      OpenGL::Scoped::deferred_upload_buffers<2> _buffers;
      GLuint const& _vertices_vbo = _buffers[0];
      GLuint const& _indices_vbo = _buffers[1];
      std::unique_ptr<OpenGL::program> _program;
  };

  class Sphere
  {
  public:
      void draw(glm::mat4x4 const& mvp
          , glm::vec3 const& pos
          , glm::vec4  const& color
          , float radius
          , int longitude = 32
          , int latitude = 18
          , float alpha = 1.f
          , bool wireframe = false
          , bool both = false
             );
    void unload();

  private:
    bool _buffers_are_setup = false;

    void setup_buffers(int longitude, int latitude);

    int _indice_count = 0;

    OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
    OpenGL::Scoped::deferred_upload_buffers<2> _buffers;
    GLuint const& _vertices_vbo = _buffers[0];
    GLuint const& _indices_vbo = _buffers[1];
    std::unique_ptr<OpenGL::program> _program;
  };

  class Square
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

    OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
    OpenGL::Scoped::deferred_upload_buffers<2> _buffers;
    GLuint const& _vertices_vbo = _buffers[0];
    GLuint const& _indices_vbo = _buffers[1];
    std::unique_ptr<OpenGL::program> _program;
  };

  /*class Cylinder
  {
  public:
      void draw(glm::mat4x4 const& mvp, glm::vec3 const& pos, const glm::vec4 color, float radius, int precision, World* world, int height = 10);
      void unload();

  private:
      bool _buffers_are_setup = false;
      void setup_buffers(int precision, World* world, int height);
      int _indice_count = 0;

      OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
      OpenGL::Scoped::deferred_upload_buffers<2> _buffers;
      GLuint const& _vertices_vbo = _buffers[0];
      GLuint const& _indices_vbo = _buffers[1];
      std::unique_ptr<OpenGL::program> _program;
  };*/

  class Line
  {
  public:
      void initSpline();
      void draw(glm::mat4x4 const& mvp, std::vector<glm::vec3> const points, glm::vec4 const& color, bool spline);
      void unload();

  private:
      bool _buffers_are_setup = false;
      void setup_buffers(std::vector<glm::vec3> const points);

      void setup_buffers_interpolated(std::vector<glm::vec3> const points);
      glm::vec3 interpolate(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 m0, glm::vec3 m1);

      int _indice_count = 0;

      void setup_shader(std::vector<glm::vec3> vertices, std::vector<std::uint16_t> indices);
      OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
      OpenGL::Scoped::deferred_upload_buffers<2> _buffers;
      GLuint const& _vertices_vbo = _buffers[0];
      GLuint const& _indices_vbo = _buffers[1];
      std::unique_ptr<OpenGL::program> _program;
  };

}
