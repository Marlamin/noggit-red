// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "FlightBoundsRender.hpp"
#include <noggit/MapTile.h>

#include <array>

using namespace Noggit::Rendering;

FlightBoundsRender::FlightBoundsRender(MapTile* map_tile)
: _map_tile(map_tile)
{

}

void FlightBoundsRender::draw(OpenGL::Scoped::use_program& mfbo_shader)
{
  static constexpr std::array<std::uint8_t, 18> indices = {4, 1, 2, 5, 8, 7, 6, 3, 0, 1, 0, 3, 6, 7, 8, 5, 2, 1};

  if (!_uploaded)
  {
    upload();

    gl.bufferData<GL_ARRAY_BUFFER>( _mfbo_bottom_vbo
        , 9 * sizeof(glm::vec3)
        , _map_tile->mMinimumValues
        , GL_STATIC_DRAW
    );
    gl.bufferData<GL_ARRAY_BUFFER>( _mfbo_top_vbo
        , 9 * sizeof(glm::vec3)
        , _map_tile->mMaximumValues
        , GL_STATIC_DRAW
    );


    {
      OpenGL::Scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> ibo_binder(_mfbo_indices);
      gl.bufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
    }

    {
      OpenGL::Scoped::vao_binder const _ (_mfbo_bottom_vao);
      OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const vbo_binder (_mfbo_bottom_vbo);
      mfbo_shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    {
      OpenGL::Scoped::vao_binder const _(_mfbo_top_vao);
      OpenGL::Scoped::buffer_binder<GL_ARRAY_BUFFER> const vbo_binder(_mfbo_top_vbo);
      mfbo_shader.attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

  }

  {
    OpenGL::Scoped::vao_binder const _(_mfbo_bottom_vao);
    OpenGL::Scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> ibo_binder(_mfbo_indices);

    mfbo_shader.uniform("color", glm::vec4(1.0f, 1.0f, 0.0f, 0.2f));
    gl.drawElements(GL_TRIANGLE_FAN, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_BYTE, nullptr);
  }

  {
    OpenGL::Scoped::vao_binder const _(_mfbo_top_vao);
    OpenGL::Scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> ibo_binder(_mfbo_indices);

    mfbo_shader.uniform("color", glm::vec4(0.0f, 1.0f, 1.0f, 0.2f));
    gl.drawElements(GL_TRIANGLE_FAN, static_cast<int>(indices.size()), GL_UNSIGNED_BYTE, nullptr);
  }

}

void FlightBoundsRender::upload()
{
  _mfbo_vbos.upload();
  _mfbo_vaos.upload();

  _uploaded = true;
}

void FlightBoundsRender::unload()
{
  _mfbo_vbos.unload();
  _mfbo_vaos.unload();

  _uploaded = false;
}