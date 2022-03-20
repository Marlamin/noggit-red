// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/rendering/Primitives.hpp>

#include <math/bounding_box.hpp>
#include <noggit/Misc.h>
#include <opengl/scoped.hpp>
#include <opengl/context.hpp>
#include <opengl/types.hpp>

#include <array>
#include <vector>
#include <glm/gtx/quaternion.hpp>

using namespace Noggit::Rendering::Primitives;

void WireBox::draw ( glm::mat4x4 const& model_view
                    , glm::mat4x4 const& projection
                    , glm::mat4x4 const& transform
                    , glm::vec4  const& color
                    , glm::vec3 const& min_point
                    , glm::vec3 const& max_point
                    )
{

  if (!_buffers_are_setup)
  {
    setup_buffers();
  }

  OpenGL::Scoped::use_program wire_box_shader {*_program.get()};

  auto points = math::box_points(min_point, max_point);

  auto glmPoints = std::vector<glm::vec3>();

  for(auto const point : points)
    {
        glmPoints.push_back(glm::vec3(point.x, point.y, point.z));
    }

  wire_box_shader.uniform("model_view", model_view);
  wire_box_shader.uniform("projection", projection);
  wire_box_shader.uniform("transform", transform);
  wire_box_shader.uniform("color", color);
  wire_box_shader.uniform("pointPositions", glmPoints);

  OpenGL::Scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
  gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  OpenGL::Scoped::vao_binder const _(_vao[0]);

  gl.drawElements (GL_LINE_STRIP, _indices, 16, GL_UNSIGNED_BYTE, nullptr);
}

void WireBox::setup_buffers()
{
  _program.reset(new OpenGL::program( {{ GL_VERTEX_SHADER, OpenGL::shader::src_from_qrc("wire_box_vs") }
                 , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("wire_box_fs")}}));

  _vao.upload();
  _buffers.upload();

  //std::vector<glm::vec3> positions (math::box_points (min_point, max_point));

  static std::array<std::uint8_t, 16> const indices
      {{5, 7, 3, 2, 0, 1, 3, 1, 5, 4, 0, 4, 6, 2, 6, 7}};

  OpenGL::Scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const index_buffer (_indices);
  gl.bufferData ( GL_ELEMENT_ARRAY_BUFFER
      , indices.size() * sizeof (*indices.data())
      , indices.data()
      , GL_STATIC_DRAW
  );

  OpenGL::Scoped::use_program shader (*_program.get());

  OpenGL::Scoped::vao_binder const _ (_vao[0]);

  _buffers_are_setup = true;

}

  void WireBox::unload()
  {
    _vao.unload();
    _buffers.unload();
    _program.reset();

    _buffers_are_setup = false;
  }

  void Grid::draw(glm::mat4x4 const& mvp
      , glm::vec3 const& pos
      , glm::vec4  const& color
      , float radius
  )
  {
    if (!_buffers_are_setup)
    {
      setup_buffers();
    }

    OpenGL::Scoped::use_program sphere_shader {*_program.get()};

    sphere_shader.uniform("model_view_projection", mvp);
    sphere_shader.uniform("origin", glm::vec3(pos.x,pos.y,pos.z));
    sphere_shader.uniform("color", color);
    sphere_shader.uniform("radius", radius);

    OpenGL::Scoped::vao_binder const _(_vao[0]);
    gl.drawElements(GL_LINES, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
  }


  void Grid::setup_buffers()
  {
    _vao.upload();
    _buffers.upload();

    _program.reset(new OpenGL::program({{ GL_VERTEX_SHADER
                                            , OpenGL::shader::src_from_qrc("grid_vs")
                                        }
                                           , { GL_FRAGMENT_SHADER
                                            , OpenGL::shader::src_from_qrc("grid_fs")
                                        }
                                       }));


    std::vector<glm::vec3> vertices;
    std::vector<std::uint16_t> indices;

    int slices = 20;

    for(int j = 0; j <= slices; ++j)
    {
      for(int i = 0; i <= slices; ++i)
      {
        float x = static_cast<float>(i) / static_cast<float>(slices);
        float y = 0;
        float z = static_cast<float>(j) / static_cast<float>(slices);
        vertices.push_back(glm::vec3(x, y, z));
      }
    }

    for(int j = 0; j < slices; ++j)
    {
      for(int i = 0; i < slices; ++i)
      {

        int row1 =  j * (slices + 1);
        int row2 = (j + 1) * (slices + 1);

        indices.push_back(row1 + i);
        indices.push_back(row1 + i + 1);
        indices.push_back(row1 + i + 1);
        indices.push_back(row2 + i + 1);

        indices.push_back(row2 + i + 1);
        indices.push_back(row2 + i);
        indices.push_back(row2 + i);
        indices.push_back(row1 + i);

      }
    }

    _indice_count = indices.size();

    gl.bufferData<GL_ARRAY_BUFFER, glm::vec3>
        (_vertices_vbo, vertices, GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>
        (_indices_vbo, indices, GL_STATIC_DRAW);


    OpenGL::Scoped::index_buffer_manual_binder indices_binder(_indices_vbo);

    OpenGL::Scoped::use_program shader (*_program.get());

    {
      OpenGL::Scoped::vao_binder const _ (_vao[0]);

      OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_vbo);
      shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }

    _buffers_are_setup = true;
  }

  void Grid::unload()
  {
    _vao.unload();
    _buffers.unload();
    _program.reset();

    _buffers_are_setup = false;
  }


  void Sphere::draw(glm::mat4x4 const& mvp
                 , glm::vec3 const& pos
                 , glm::vec4  const& color
                 , float radius
                 )
{
  if (!_buffers_are_setup)
  {
    setup_buffers();
  }

  OpenGL::Scoped::use_program sphere_shader {*_program.get()};

  sphere_shader.uniform("model_view_projection", mvp);
  sphere_shader.uniform("origin", glm::vec3(pos.x,pos.y,pos.z));
  sphere_shader.uniform("radius", radius);
  sphere_shader.uniform("color", color);

  OpenGL::Scoped::vao_binder const _(_vao[0]);
  gl.drawElements(GL_TRIANGLES, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
}


void Sphere::setup_buffers()
{
  _vao.upload();
  _buffers.upload();

  std::vector<glm::vec3> vertices;
  std::vector<std::uint16_t> indices;

  _program.reset(new OpenGL::program({{ GL_VERTEX_SHADER
               , OpenGL::shader::src_from_qrc("sphere_vs")}
             , { GL_FRAGMENT_SHADER
               , OpenGL::shader::src_from_qrc("sphere_fs")
               }}));


  int segment = 27;

  // add overlapping vertices at the end for an easier vertices generations
  for (int rotation_step = 0; rotation_step <= segment; ++rotation_step)
  {
    math::degrees rotation(360.f*rotation_step / static_cast<float>(segment));
    auto rotationQuat = glm::angleAxis(rotation._, glm::vec3(0, 0, 1));
    for (int i = 0; i < segment; ++i)
    {
      float x = glm::cos(math::radians(math::degrees(i * 360 / segment))._);
      float z = glm::sin(math::radians(math::degrees(i * 360 / segment))._);

      glm::vec4 v(x, 0.f, z,0.0f);

      vertices.emplace_back(glm::toMat4(rotationQuat) * v);

      if (rotation_step < segment)
      {
        indices.emplace_back(i + rotation_step*segment);
        indices.emplace_back(((i + 1) % segment) + rotation_step * segment);
        indices.emplace_back(i + (rotation_step+1) * segment);

        indices.emplace_back(i + (rotation_step+1) * segment);
        indices.emplace_back(((i + 1) % segment) + rotation_step * segment);
        indices.emplace_back(((i + 1) % segment) + (rotation_step+1) * segment);
      }
    }
  }

  _indice_count = indices.size();

  gl.bufferData<GL_ARRAY_BUFFER, glm::vec3>
    (_vertices_vbo, vertices, GL_STATIC_DRAW);
  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>
    (_indices_vbo, indices, GL_STATIC_DRAW);


  OpenGL::Scoped::index_buffer_manual_binder indices_binder(_indices_vbo);

  OpenGL::Scoped::use_program shader (*_program.get());

  {
    OpenGL::Scoped::vao_binder const _ (_vao[0]);

    OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_vbo);
    shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

    indices_binder.bind();
  }

  _buffers_are_setup = true;
}

  void Sphere::unload()
  {
    _vao.unload();
    _buffers.unload();
    _program.reset();

    _buffers_are_setup = false;
  }


  void Square::draw(glm::mat4x4 const& mvp
                 , glm::vec3 const& pos
                 , float radius
                 , math::radians inclination
                 , math::radians orientation
                 , glm::vec4  const& color
                 )
{
  if (!_buffers_are_setup)
  {
    setup_buffers();
  }

  OpenGL::Scoped::use_program sphere_shader {*_program.get()};

  sphere_shader.uniform("model_view_projection", mvp);
  sphere_shader.uniform("origin", glm::vec3(pos.x,pos.y,pos.z));
  sphere_shader.uniform("radius", radius);
  sphere_shader.uniform("inclination", inclination._);
  sphere_shader.uniform("orientation", orientation._);
  sphere_shader.uniform("color", color);

  OpenGL::Scoped::vao_binder const _ (_vao[0]);
  gl.drawElements(GL_TRIANGLES, _indices_vbo, 6, GL_UNSIGNED_SHORT, nullptr);
}


void Square::setup_buffers()
{
  _vao.upload();
  _buffers.upload();

  std::vector<glm::vec3> vertices =
  {
     {-1.f, 0.f, -1.f}
    ,{-1.f, 0.f,  1.f}
    ,{ 1.f, 0.f,  1.f}
    ,{ 1.f, 0.f, -1.f}
  };
  std::vector<std::uint16_t> indices = {0,1,2, 2,3,0};

  _program.reset(new OpenGL::program({{ GL_VERTEX_SHADER
               , OpenGL::shader::src_from_qrc("square_vs")
               }, { GL_FRAGMENT_SHADER
               , OpenGL::shader::src_from_qrc("square_fs")
               }}));


  gl.bufferData<GL_ARRAY_BUFFER, glm::vec3>
    (_vertices_vbo, vertices, GL_STATIC_DRAW);
  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>
    (_indices_vbo, indices, GL_STATIC_DRAW);


  OpenGL::Scoped::index_buffer_manual_binder indices_binder (_indices_vbo);

  OpenGL::Scoped::use_program shader(*_program.get());

  {
    OpenGL::Scoped::vao_binder const _ (_vao[0]);

    OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_vbo);
    shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

    indices_binder.bind();
  }

  _buffers_are_setup = true;
}

  void Square::unload()
  {
    _vao.unload();
    _buffers.unload();
    _program.reset();

    _buffers_are_setup = false;

  }

