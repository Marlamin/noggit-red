// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncObject.h>
#include <noggit/ContextObject.hpp>
#include <noggit/multimap_with_normalized_key.hpp>
#include <opengl/texture.hpp>
#include <opengl/context.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFramebufferObjectFormat>
#include <QtOpenGL/QGLPixelBuffer>

#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>


struct BLPHeader;

struct scoped_blp_texture_reference;
struct blp_texture : public opengl::texture, AsyncObject
{
  blp_texture (std::string const& filename, noggit::NoggitRenderContext context);
  void finishLoading();

  void loadFromUncompressedData(BLPHeader const* lHeader, char const* lData);
  void loadFromCompressedData(BLPHeader const* lHeader, char const* lData);

  int width() const { return _width; }
  int height() const { return _height; }

  void bind();
  void upload();
  void unload();

  noggit::NoggitRenderContext getContext() { return _context; };

  virtual async_priority loading_priority() const
  {
    return async_priority::high;
  }

private:
  bool _uploaded = false;

  int _width;
  int _height;

  noggit::NoggitRenderContext _context;

private:
  std::map<int, std::vector<uint32_t>> _data;
  std::map<int, std::vector<uint8_t>> _compressed_data;
  boost::optional<GLint> _compression_format;
};

class TextureManager
{
public:
  static void report();
  static void unload_all(noggit::NoggitRenderContext context);

private:
  friend struct scoped_blp_texture_reference;
  static noggit::async_object_multimap_with_normalized_key<blp_texture> _;
};

struct scoped_blp_texture_reference
{
  scoped_blp_texture_reference() = delete;
  scoped_blp_texture_reference (std::string const& filename, noggit::NoggitRenderContext context);
  scoped_blp_texture_reference (scoped_blp_texture_reference const& other);
  scoped_blp_texture_reference (scoped_blp_texture_reference&&) = default;
  scoped_blp_texture_reference& operator= (scoped_blp_texture_reference const&) = delete;
  scoped_blp_texture_reference& operator= (scoped_blp_texture_reference&&) = default;
  ~scoped_blp_texture_reference() = default;

  blp_texture* operator->() const;
  blp_texture* get() const;

  bool operator== (scoped_blp_texture_reference const& other) const;

private:
  struct Deleter
  {
    void operator() (blp_texture*) const;
  };
  std::unique_ptr<blp_texture, Deleter> _blp_texture;
  noggit::NoggitRenderContext _context;
};

namespace noggit
{

  class BLPRenderer
  {
  private:
    BLPRenderer();

    BLPRenderer( const BLPRenderer&);
    BLPRenderer& operator=( BLPRenderer& );

    std::map<std::tuple<std::string, int, int>, QPixmap> _cache;

    QOpenGLContext _context;
    QOpenGLFramebufferObjectFormat _fmt;
    QOffscreenSurface _surface;
    std::unique_ptr<opengl::program> _program;

    opengl::scoped::deferred_upload_vertex_arrays<1> _vao;
    opengl::scoped::deferred_upload_buffers<3> _buffers;

  public:
    static BLPRenderer& getInstance()
    {
      static BLPRenderer  instance;
      return instance;
    }

    ~BLPRenderer();

    QPixmap* render_blp_to_pixmap ( std::string const& blp_filename, int width = -1, int height = -1);

  };


}
