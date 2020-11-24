// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>
#include <math/matrix_4x4.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Misc.h>
#include <opengl/context.hpp>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>
#include <opengl/types.hpp>

#include <array>
#include <vector>

namespace opengl
{
  namespace primitives
  {
    void wire_box::draw ( math::matrix_4x4 const& model_view
                        , math::matrix_4x4 const& projection
                        , math::matrix_4x4 const& transform
                        , math::vector_4d const& color
                        , math::vector_3d const& min_point
                        , math::vector_3d const& max_point
                        )
    {

      if (!_buffers_are_setup)
      {
        setup_buffers();
      }

      opengl::scoped::use_program wire_box_shader {*_program.get()};

      wire_box_shader.uniform("model_view", model_view);
      wire_box_shader.uniform("projection", projection);
      wire_box_shader.uniform("transform", transform);
      wire_box_shader.uniform("color", color);
      wire_box_shader.uniform("pointPositions", math::box_points (min_point, max_point));

      opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      
      opengl::scoped::vao_binder const _(_vao[0]);

      gl.drawElements (GL_LINE_STRIP, _indices, 16, GL_UNSIGNED_BYTE, nullptr);
    }

    void wire_box::setup_buffers()
    {
      _program.reset(new opengl::program( {{ GL_VERTEX_SHADER
                     , R"code(
                          #version 330 core

                          uniform vec3 pointPositions[8];
                          uniform mat4 model_view;
                          uniform mat4 projection;
                          uniform mat4 transform;

                          void main()
                          {
                            gl_Position = projection * model_view * transform * vec4(pointPositions[gl_VertexID], 1.0);
                          }
                          )code"}
                     , { GL_FRAGMENT_SHADER
                     , R"code(
                          #version 330 core

                          uniform vec4 color;

                          out vec4 out_color;

                          void main()
                          {
                            out_color = color;
                          }
                          )code"
                     }
      }));

      _vao.upload();
      _buffers.upload();

      //std::vector<math::vector_3d> positions (math::box_points (min_point, max_point));

      static std::array<std::uint8_t, 16> const indices
          {{5, 7, 3, 2, 0, 1, 3, 1, 5, 4, 0, 4, 6, 2, 6, 7}};

      scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const index_buffer (_indices);
      gl.bufferData ( GL_ELEMENT_ARRAY_BUFFER
          , indices.size() * sizeof (*indices.data())
          , indices.data()
          , GL_STATIC_DRAW
      );

      opengl::scoped::use_program shader (*_program.get());

      opengl::scoped::vao_binder const _ (_vao[0]);

      _buffers_are_setup = true;

    }

      void wire_box::unload()
      {
        _vao.unload();
        _buffers.unload();
        _program.reset();

        _buffers_are_setup = false;
      }

      void grid::draw( math::matrix_4x4 const& mvp
          , math::vector_3d const& pos
          , math::vector_4d const& color
          , float radius
      )
      {
        if (!_buffers_are_setup)
        {
          setup_buffers();
        }

        opengl::scoped::use_program sphere_shader {*_program.get()};

        sphere_shader.uniform("model_view_projection", mvp);
        sphere_shader.uniform("origin", pos);
        sphere_shader.uniform("color", color);
        sphere_shader.uniform("radius", radius);

        opengl::scoped::vao_binder const _(_vao[0]);
        gl.drawElements(GL_LINES, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
      }


      void grid::setup_buffers()
      {
        _vao.upload();
        _buffers.upload();

        _program.reset(new opengl::program({{ GL_VERTEX_SHADER
                                                , R"code(
                                                #version 330 core

                                                in vec4 position;

                                                uniform mat4 model_view_projection;
                                                uniform vec3 origin;
                                                uniform float radius;

                                                void main()
                                                {
                                                  vec4 pos = position;
                                                  pos.xyz *= radius;

                                                  vec3 origin_fixed = vec3(origin.x - radius / 2.0f, origin.y, origin.z - radius / 2.0f);
                                                  pos.xyz += origin_fixed;
                                                  gl_Position = model_view_projection * pos;
                                                }
                                                )code"
                                            }
                                               , { GL_FRAGMENT_SHADER
                                                , R"code(
                                                #version 330 core

                                                uniform vec4 color;

                                                out vec4 out_color;

                                                void main()
                                                {
                                                  out_color = color;
                                                }
                                                )code"
                                            }
                                           }));


        std::vector<math::vector_3d> vertices;
        std::vector<std::uint16_t> indices;

        int slices = 20;

        for(int j = 0; j <= slices; ++j)
        {
          for(int i = 0; i <= slices; ++i)
          {
            float x = static_cast<float>(i) / static_cast<float>(slices);
            float y = 0;
            float z = static_cast<float>(j) / static_cast<float>(slices);
            vertices.push_back(math::vector_3d(x, y, z));
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

        gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>
            (_vertices_vbo, vertices, GL_STATIC_DRAW);
        gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>
            (_indices_vbo, indices, GL_STATIC_DRAW);


        scoped::index_buffer_manual_binder indices_binder(_indices_vbo);

        opengl::scoped::use_program shader (*_program.get());

        {
          opengl::scoped::vao_binder const _ (_vao[0]);

          opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_vbo);
          shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

          indices_binder.bind();
        }

        _buffers_are_setup = true;
      }

      void grid::unload()
      {
        _vao.unload();
        _buffers.unload();
        _program.reset();

        _buffers_are_setup = false;
      }


      void sphere::draw( math::matrix_4x4 const& mvp
                     , math::vector_3d const& pos
                     , math::vector_4d const& color
                     , float radius
                     )
    {
      if (!_buffers_are_setup)
      {
        setup_buffers();
      }

      opengl::scoped::use_program sphere_shader {*_program.get()};

      sphere_shader.uniform("model_view_projection", mvp);
      sphere_shader.uniform("origin", pos);
      sphere_shader.uniform("radius", radius);
      sphere_shader.uniform("color", color);

      opengl::scoped::vao_binder const _(_vao[0]);
      gl.drawElements(GL_TRIANGLES, _indices_vbo, _indice_count, GL_UNSIGNED_SHORT, nullptr);
    }
  

    void sphere::setup_buffers()
    {
      _vao.upload();
      _buffers.upload();

      std::vector<math::vector_3d> vertices;
      std::vector<std::uint16_t> indices;

      _program.reset(new opengl::program({{ GL_VERTEX_SHADER
                   , R"code(
#version 330 core

in vec4 position;

uniform mat4 model_view_projection;
uniform vec3 origin;
uniform float radius;

void main()
{
  vec4 pos = position;
  pos.xyz *= radius;
  pos.xyz += origin;
  gl_Position = model_view_projection * pos;
}
)code"
                   }
                 , { GL_FRAGMENT_SHADER
                   , R"code(
#version 330 core

uniform vec4 color;

out vec4 out_color;

void main()
{
  out_color = color;
}
)code"
                   }
                     }));


      int segment = 27;

      // add overlapping vertices at the end for an easier vertices generations
      for (int rotation_step = 0; rotation_step <= segment; ++rotation_step)
      {
        math::degrees rotation(360.f*rotation_step / static_cast<float>(segment));
        math::matrix_4x4 m(math::matrix_4x4::rotation_xyz, math::degrees::vec3(math::degrees(0.f), math::degrees(0.f), rotation));

        for (int i = 0; i < segment; ++i)
        {
          float x = math::cos(math::degrees(i * 360 / segment));
          float z = math::sin(math::degrees(i * 360 / segment));

          math::vector_3d v(x, 0.f, z);

          vertices.emplace_back(m*v);

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

      gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>
        (_vertices_vbo, vertices, GL_STATIC_DRAW);
      gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>
        (_indices_vbo, indices, GL_STATIC_DRAW);


      scoped::index_buffer_manual_binder indices_binder(_indices_vbo);

      opengl::scoped::use_program shader (*_program.get());

      {
        opengl::scoped::vao_binder const _ (_vao[0]);

        opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_vbo);
        shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

        indices_binder.bind();
      }

      _buffers_are_setup = true;
    }

      void sphere::unload()
      {
        _vao.unload();
        _buffers.unload();
        _program.reset();

        _buffers_are_setup = false;
      }


      void square::draw( math::matrix_4x4 const& mvp
                     , math::vector_3d const& pos
                     , float radius
                     , math::radians inclination
                     , math::radians orientation
                     , math::vector_4d const& color
                     )
    {
      if (!_buffers_are_setup)
      {
        setup_buffers();
      }

      opengl::scoped::use_program sphere_shader {*_program.get()};

      sphere_shader.uniform("model_view_projection", mvp);
      sphere_shader.uniform("origin", pos);
      sphere_shader.uniform("radius", radius);
      sphere_shader.uniform("inclination", inclination._);
      sphere_shader.uniform("orientation", orientation._);
      sphere_shader.uniform("color", color);

      opengl::scoped::vao_binder const _ (_vao[0]);
      gl.drawElements(GL_TRIANGLES, _indices_vbo, 6, GL_UNSIGNED_SHORT, nullptr);
    }


    void square::setup_buffers()
    {
      _vao.upload();
      _buffers.upload();

      std::vector<math::vector_3d> vertices = 
      {
         {-1.f, 0.f, -1.f}
        ,{-1.f, 0.f,  1.f}
        ,{ 1.f, 0.f,  1.f}
        ,{ 1.f, 0.f, -1.f}
      };
      std::vector<std::uint16_t> indices = {0,1,2, 2,3,0};

      _program.reset(new opengl::program({{ GL_VERTEX_SHADER
                   , R"code(
#version 330 core

in vec4 position;

uniform mat4 model_view_projection;
uniform vec3 origin;
uniform float radius;
uniform float inclination;
uniform float orientation;

void main()
{
  vec4 pos = position;
  float cos_o = cos(orientation);
  float sin_o = sin(orientation);

  pos.y += pos.x * tan(inclination) * radius;

  pos.x = (position.x*cos_o - position.z * sin_o) * radius;
  pos.z = (position.z*cos_o + position.x * sin_o) * radius;

  pos.xyz += origin;
  gl_Position = model_view_projection * pos;
}
)code"
                   }
                 , { GL_FRAGMENT_SHADER
                   , R"code(
#version 330 core

uniform vec4 color;

out vec4 out_color;

void main()
{
  out_color = color;
}
)code"
                   }
                     }));


      gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>
        (_vertices_vbo, vertices, GL_STATIC_DRAW);
      gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>
        (_indices_vbo, indices, GL_STATIC_DRAW);


      scoped::index_buffer_manual_binder indices_binder (_indices_vbo);

      opengl::scoped::use_program shader(*_program.get());

      {
        opengl::scoped::vao_binder const _ (_vao[0]);

        opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const vertices_binder (_vertices_vbo);
        shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

        indices_binder.bind();
      }

      _buffers_are_setup = true;
    }

      void square::unload()
      {
        _vao.unload();
        _buffers.unload();
        _program.reset();

        _buffers_are_setup = false;

      }
  }
}
