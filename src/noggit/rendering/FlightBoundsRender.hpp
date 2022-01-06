// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_FLIGHTBOUNDSRENDER_HPP
#define NOGGIT_FLIGHTBOUNDSRENDER_HPP

#include <noggit/rendering/BaseRender.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

class MapTile;

namespace Noggit::Rendering
{
  class FlightBoundsRender : public BaseRender
  {
  public:
    FlightBoundsRender(MapTile* map_tile);

    void upload() override;
    void unload() override;

    void draw(OpenGL::Scoped::use_program&);

  private:
    MapTile* _map_tile;

    bool _uploaded = false;

    OpenGL::Scoped::deferred_upload_vertex_arrays<2> _mfbo_vaos;
    GLuint const& _mfbo_bottom_vao = _mfbo_vaos[0];
    GLuint const& _mfbo_top_vao = _mfbo_vaos[1];
    OpenGL::Scoped::deferred_upload_buffers<3> _mfbo_vbos;
    GLuint const& _mfbo_bottom_vbo = _mfbo_vbos[0];
    GLuint const& _mfbo_top_vbo = _mfbo_vbos[1];
    GLuint const& _mfbo_indices = _mfbo_vbos[2];

  };
}

#endif //NOGGIT_FLIGHTBOUNDSRENDER_HPP
