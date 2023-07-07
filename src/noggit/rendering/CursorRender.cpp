﻿// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "CursorRender.hpp"
#include "math/trig.hpp"
#include "opengl/shader.hpp"

namespace Noggit
{
  void CursorRender::draw(Mode cursor_mode, glm::mat4x4 const& mvp, glm::vec4 color, glm::vec3 const& pos, float radius, float inner_radius_ratio)
  {
    if (!_uploaded)
    {
      upload();
    }

    OpenGL::Scoped::use_program shader {*_cursor_program.get()};

    shader.uniform("model_view_projection", mvp);
    shader.uniform("cursor_pos", glm::vec3(pos.x,pos.y,pos.z));
    shader.uniform("color", color);
    shader.uniform("radius", radius);

    OpenGL::Scoped::vao_binder const _ (_vaos[static_cast<int>(cursor_mode)]);

    gl.drawElements(GL_LINES, _indices_count[cursor_mode], GL_UNSIGNED_SHORT, nullptr);

    if (inner_radius_ratio > 0.f)
    {
      shader.uniform("radius", radius*inner_radius_ratio);
      gl.drawElements(GL_LINES, _indices_count[cursor_mode], GL_UNSIGNED_SHORT, nullptr);
    }
  }

  void CursorRender::upload()
  {
    _vaos.upload();
    _vbos.upload();

    _cursor_program.reset
      ( new OpenGL::program
          { { GL_VERTEX_SHADER,   OpenGL::shader::src_from_qrc("cursor_vs") }
          , { GL_FRAGMENT_SHADER, OpenGL::shader::src_from_qrc("cursor_fs") }
          }
      );    

    OpenGL::Scoped::use_program shader {*_cursor_program.get()};

    create_circle_buffer(shader);
    create_sphere_buffer(shader);
    create_square_buffer(shader);
    create_cube_buffer(shader);

    _uploaded = true;
  }

  void CursorRender::create_circle_buffer(OpenGL::Scoped::use_program& shader)
  {
    std::vector<glm::vec3> vertices;
    std::vector<std::uint16_t> indices;

    int segment = 60;

    for (int i = 0; i < segment; ++i)
    {
      float x = glm::cos(math::radians(math::degrees(i * 360 / segment))._);
      float z = glm::sin(math::radians(math::degrees(i * 360 / segment))._);
      vertices.emplace_back(x, 0.f, z);
      indices.emplace_back(i);
      indices.emplace_back((i + 1) % segment);
    }

    _indices_count[Mode::circle] = static_cast<int>(indices.size());

    int id = static_cast<int>(Mode::circle);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    OpenGL::Scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      OpenGL::Scoped::vao_binder const _(_vaos[id]);

      OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> vertices_vbo(_vbos[id * 2]);
      shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }
  }

  void CursorRender::create_sphere_buffer(OpenGL::Scoped::use_program& shader)
  {
    std::vector<glm::vec3> vertices;
    std::vector<std::uint16_t> indices;

    int segment = 60;
    int rotation_plane = 6;

    int id_ofs = 0;

    for (int r = 0; r <= rotation_plane; ++r)
    {
    	math::degrees rotation(360.f*r / static_cast<float>(rotation_plane));

        glm::mat4x4 rotationMatrix = glm::mat4x4();
        glm::mat4x4 m = glm::rotate(rotationMatrix, math::radians(rotation)._, glm::vec3(0, 0, 1));


      for (int i = 0; i < segment; ++i)
      {
        float x = glm::cos(math::radians(math::degrees(i * 360 / segment))._);
        float z = glm::sin(math::radians(math::degrees(i * 360 / segment))._);

        glm::vec4 v(x, 0.f, z,0);

        vertices.emplace_back(m * v);
        if (r < rotation_plane)
        {
          indices.emplace_back(i + id_ofs);
          indices.emplace_back(((i + 1) % segment) + id_ofs);
        }
      }

      id_ofs += segment;
    }
    
    for (int i = 0; i < segment; ++i)
    {
      for (int r = 0; r < rotation_plane; ++r)
      {
        indices.emplace_back(i + r*segment);
        indices.emplace_back(i + (r+1)*segment);
      }
    }    

    _indices_count[Mode::sphere] = static_cast<int>(indices.size());

    int id = static_cast<int>(Mode::sphere);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    OpenGL::Scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      OpenGL::Scoped::vao_binder const _(_vaos[id]);

      OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> vertices_vbo(_vbos[id * 2]);
      shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }
  }

  void CursorRender::create_square_buffer(OpenGL::Scoped::use_program& shader)
  {
    std::vector<glm::vec3> vertices = 
    {
       {-0.5f,0.f,-0.5f}
      ,{ 0.5f,0.f,-0.5f}
      ,{ 0.5f,0.f, 0.5f}
      ,{-0.5f,0.f, 0.5f}
    };

    std::vector<std::uint16_t> indices = {0,1, 1,2 ,2,3 ,3,0};    

    _indices_count[Mode::square] = static_cast<int>(indices.size());

    int id = static_cast<int>(Mode::square);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    OpenGL::Scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      OpenGL::Scoped::vao_binder const _(_vaos[id]);

      OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> vertices_vbo(_vbos[id * 2]);
      shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }

    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void CursorRender::create_cube_buffer(OpenGL::Scoped::use_program& shader)
  {
    std::vector<glm::vec3> vertices =
    {
        {-0.5f,-0.5f,-0.5f}
      , { 0.5f,-0.5f,-0.5f}
      , { 0.5f,-0.5f, 0.5f}
      , {-0.5f,-0.5f, 0.5f}
      , {-0.5f, 0.5f,-0.5f}
      , { 0.5f, 0.5f,-0.5f}
      , { 0.5f, 0.5f, 0.5f}
      , {-0.5f, 0.5f, 0.5f}
    };

    std::vector<std::uint16_t> indices = 
    {  
        0,1, 1,2 ,2,3 ,3,0
      , 0,4, 1,5 ,2,6 ,3,7
      , 4,5, 5,6 ,6,7 ,7,4
    };

    _indices_count[Mode::cube] = static_cast<int>(indices.size());

    int id = static_cast<int>(Mode::cube);

    gl.bufferData<GL_ARRAY_BUFFER>(_vbos[id * 2], vertices.size() * sizeof(*vertices.data()), vertices.data(), GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>(_vbos[id * 2 + 1], indices.size() * sizeof(*indices.data()), indices.data(), GL_STATIC_DRAW);

    OpenGL::Scoped::index_buffer_manual_binder indices_binder(_vbos[id * 2 + 1]);

    {
      OpenGL::Scoped::vao_binder const _(_vaos[id]);

      OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> vertices_vbo(_vbos[id * 2]);
      shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

      indices_binder.bind();
    }

    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

    void CursorRender::unload()
    {
      _vaos.unload();
      _vbos.unload();

      _cursor_program.reset();

      _uploaded = false;
    }
}
