// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/rendering/Primitives.hpp>

#include <math/bounding_box.hpp>
#include <noggit/Misc.h>
#include <opengl/scoped.hpp>
#include <opengl/context.hpp>
#include <opengl/types.hpp>
#include <noggit/World.h>

#include <numbers>
#include <array>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

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


  void Sphere::draw(glm::mat4x4 const& mvp, glm::vec3 const& pos, glm::vec4  const& color
      , float radius, int longitude, int latitude, float alpha, bool wireframe, bool drawBoth)
{
  if (!_buffers_are_setup)
  {
    setup_buffers(longitude, latitude);
  }

  OpenGL::Scoped::use_program sphere_shader {*_program.get()};

  sphere_shader.uniform("model_view_projection", mvp);
  sphere_shader.uniform("origin", glm::vec3(pos.x,pos.y,pos.z));
  sphere_shader.uniform("radius", radius);
  sphere_shader.uniform("color", glm::vec4(color.r, color.g, color.b, alpha));

  OpenGL::Scoped::vao_binder const _(_vao[0]);
  if (drawBoth)
  {
      gl.drawElements(GL_TRIANGLES, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
      sphere_shader.uniform("color", glm::vec4(1.f, 1.f, 1.f, 1.f));
      gl.drawElements(GL_LINE_STRIP, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
      return;
  }

  if (!wireframe)
  {
        gl.drawElements(GL_TRIANGLES, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
  }
  else
  {
        gl.drawElements(GL_LINE_STRIP, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
  }
}


void Sphere::setup_buffers(int longitude, int latitude)
{
  _vao.upload();
  _buffers.upload();

  const int na = longitude;
  const int nb = latitude;
  const int na3 = na * 3;
  const int nn = nb * na3;

  std::vector<glm::vec3> vertices;
  std::vector<std::uint16_t> indices;

  _program.reset(new OpenGL::program({{ GL_VERTEX_SHADER
               , OpenGL::shader::src_from_qrc("sphere_vs")}
             , { GL_FRAGMENT_SHADER
               , OpenGL::shader::src_from_qrc("sphere_fs")
               }}));

  float x, y, z, a, b, da, db, r = 3.5f;
  int ia, ib, ix, iy;
  da = glm::two_pi<float>() / float(na);
  db = glm::pi<float>() / float(nb - 1);

  for (ix = 0, b = -glm::half_pi<float>(), ib = 0; ib < nb; ib++, b += db)
  {
      for (a = 0.f, ia = 0; ia < na; ia++, a += da, ix += 3)
      {
          x = cos(b) * cos(a);
          z = cos(b) * sin(a);
          y = sin(b);

          vertices.emplace_back(x, y, z);
      }
  }

  for (ix = 0, iy = 0, ib = 1; ib < nb; ib++)
  {
      for (ia = 1; ia < na; ia++, iy++)
      {
          indices.push_back(iy); ix++;
          indices.push_back(iy + 1); ix++;
          indices.push_back(iy + na); ix++;

          indices.push_back(iy + na); ix++;
          indices.push_back(iy + 1); ix++;
          indices.push_back(iy + na + 1); ix++;
      }

      indices.push_back(iy); ix++;
      indices.push_back(iy + 1 - na); ix++;
      indices.push_back(iy + na); ix++;

      indices.push_back(iy + na); ix++;
      indices.push_back(iy - na + 1); ix++;
      indices.push_back(iy + 1); ix++;
      iy++;
  }

  _indice_count = (int)indices.size();

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

  /*void Cylinder::draw(glm::mat4x4 const& mvp, glm::vec3 const& pos, const glm::vec4 color, float radius, int precision, World* world, int height)
  {
      if (!_buffers_are_setup)
      {
          setup_buffers(precision, world, height);
      }

      OpenGL::Scoped::use_program cylinder_shader {*_program.get()};

      cylinder_shader.uniform("model_view_projection", mvp);
      cylinder_shader.uniform("origin", glm::vec3(pos.x,pos.y,pos.z));
      cylinder_shader.uniform("radius", radius);
      cylinder_shader.uniform("color", color);
      cylinder_shader.uniform("height", height);

      OpenGL::Scoped::vao_binder const _(_vao[0]);

      gl.drawElements(GL_TRIANGLES, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
  }

  void Cylinder::unload()
  {
    _vao.unload();
    _buffers.unload();
    _program.reset();

    _buffers_are_setup = false;
  }

  void Cylinder::setup_buffers(int precision, World* world, int height)
  {
      if (height <= 10.f)
          height = 10.f;

      _vao.upload();
      _buffers.upload();

      std::vector<glm::vec3> vertices;
      std::vector<std::uint16_t> indices;

      _program.reset(new OpenGL::program({
          { GL_VERTEX_SHADER, OpenGL::shader::src_from_qrc("cylinder_vs")},
          { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("cylinder_fs")}
      }));

      int num = (precision + 1) * 2;
      int numi = precision * 6;

      vertices.resize(num);
      indices.resize(numi);

      for (int i = 0; i <= 1; i++)
      {
          for (int j = 0; j <= precision; j++)
          {
              float y = (i == 0) ? 0.f : float(height);
              float x = -(float)cos(j * glm::two_pi<float>() / precision);
              float z = (float)sin(j * glm::two_pi<float>() / precision);

              vertices[i * (precision + 1) + j] = glm::vec3(x, y, z);
          }
      }

      for (int i = 0; i < 1; i++)
      {
          for (int j = 0; j < precision; j++)
          {
              indices[6 * (i * precision + j) + 0] = i * (precision + 1) + j;
              indices[6 * (i * precision + j) + 1] = i * (precision + 1) + j + 1;
              indices[6 * (i * precision + j) + 2] = (i + 1) * (precision + 1) + j;
              indices[6 * (i * precision + j) + 3] = i * (precision + 1) + j + 1;
              indices[6 * (i * precision + j) + 4] = (i + 1) * (precision + 1) + j + 1;
              indices[6 * (i * precision + j) + 5] = (i + 1) * (precision + 1) + j;
          }
      }

      _indice_count = (int)indices.size();

      gl.bufferData<GL_ARRAY_BUFFER, glm::vec3> (_vertices_vbo, vertices, GL_STATIC_DRAW);
      gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t> (_indices_vbo, indices, GL_STATIC_DRAW);


      OpenGL::Scoped::index_buffer_manual_binder indices_binder(_indices_vbo);
      OpenGL::Scoped::use_program shader (*_program.get());

      {
          OpenGL::Scoped::vao_binder const _ (_vao[0]);
          OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_vbo);
          shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);
          indices_binder.bind();
      }

      _buffers_are_setup = true;
  }*/

  void Line::initSpline()
  {
      draw(glm::mat4x4{},
          std::vector<glm::vec3>{ {}, {} },
          glm::vec4{},
          false);
  }

  void Line::draw(glm::mat4x4 const& mvp
      , std::vector<glm::vec3> const points
      , glm::vec4 const& color
      , bool spline
  )
  {
      if (points.size() < 2)
          return;

      if (!spline || points.size() == 2)
      {
          setup_buffers(points);
      }
      else
      {
          initSpline();
          setup_buffers_interpolated(points);
      }

      OpenGL::Scoped::use_program line_shader{ *_program.get() };

      line_shader.uniform("model_view_projection", mvp);
      line_shader.uniform("color", color);

      OpenGL::Scoped::vao_binder const _(_vao[0]);
      gl.drawElements(GL_LINE_STRIP, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
  }


  void Line::setup_buffers(std::vector<glm::vec3> const points)
  {
      _vao.upload();
      _buffers.upload();

      std::vector<glm::vec3> vertices = points;
      std::vector<std::uint16_t> indices;

      for (int i = 0; i < points.size(); ++i)
      {
          indices.push_back(i);
      }

      setup_shader(vertices, indices);
  }

  void Line::setup_buffers_interpolated(std::vector<glm::vec3> const points)
  {
      const float tension = 0.5f;

      std::vector<glm::vec3> tempPoints;
      tempPoints.push_back(points[0]);

      for (auto const& p : points)
          tempPoints.push_back(p);

      tempPoints.push_back(points[points.size() - 1]);

      std::vector<glm::vec3> vertices;
      std::vector<std::uint16_t> indices;

      for (int i = 1; i < tempPoints.size() - 2; i++)
      {
          auto s = tension * 2.f;
          auto p0 = tempPoints[i - 1];
          auto p1 = tempPoints[i + 0];
          auto p2 = tempPoints[i + 1];
          auto p3 = tempPoints[i + 2];

          glm::vec3 m1(
              (p2.x - p0.x) / s,
              (p2.y - p0.y) / s,
              (p2.z - p0.z) / s
          );

          glm::vec3 m2(
              (p3.x - p1.x) / s,
              (p3.y - p1.y) / s,
              (p3.z - p1.z) / s
          );

          vertices.push_back(interpolate(0, p1, p2, m1, m2));

          for (float t = 0.01f; t < 1.f; t += 0.01f)
              vertices.push_back(interpolate(t, p1, p2, m1, m2));

          vertices.push_back(interpolate(1, p1, p2, m1, m2));
      }

      for (int i = 0; i < vertices.size(); ++i)
      {
          indices.push_back(i);
      }

      setup_shader(vertices, indices);
  }

  glm::vec3 Line::interpolate(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 m0, glm::vec3 m1)
  {
      auto c = 2 * t * t * t - 3 * t * t;
      auto c0 = c + 1;
      auto c1 = t * t * t - 2 * t * t + t;
      auto c2 = -c;
      auto c3 = t * t * t - t * t;

      return (c0 * p0 + c1 * m0 + c2 * p1 + c3 * m1);
  }

  void Line::setup_shader(std::vector<glm::vec3> vertices, std::vector<std::uint16_t> indices)
  {
      _indice_count = (int)indices.size();
      _program.reset(new OpenGL::program(
          {
              { GL_VERTEX_SHADER, OpenGL::shader::src_from_qrc("line_vs") },
              { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("line_fs") }
          }
      ));

      gl.bufferData<GL_ARRAY_BUFFER, glm::vec3>(_vertices_vbo, vertices, GL_STATIC_DRAW);
      gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_indices_vbo, indices, GL_STATIC_DRAW);

      OpenGL::Scoped::index_buffer_manual_binder indices_binder(_indices_vbo);
      OpenGL::Scoped::use_program shader(*_program.get());

      {
          OpenGL::Scoped::vao_binder const _(_vao[0]);

          OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder(_vertices_vbo);
          shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);
          indices_binder.bind();
      }

      _buffers_are_setup = true;
  }

  void Line::unload()
  {
      _vao.unload();
      _buffers.unload();
      _program.reset();

      _buffers_are_setup = false;

  }