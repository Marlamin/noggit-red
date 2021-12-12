// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncObject.h>
#include <noggit/ContextObject.hpp>
#include <noggit/AsyncObjectMultimap.hpp>
#include <opengl/texture.hpp>
#include <opengl/context.hpp>
#include <opengl/context.inl>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFramebufferObjectFormat>
#include <QtOpenGL/QGLPixelBuffer>
#include <optional>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>
#include <tuple>


struct tuple_hash
{
  template <class T1, class T2, class T3, class T4>
  std::size_t operator() (const std::tuple<T1,T2,T3,T4> &p) const
  {
    auto h1 = std::hash<T1>{}(std::get<0>(p));
    auto h2 = std::hash<T2>{}(std::get<1>(p));
    auto h3 = std::hash<T3>{}(std::get<2>(p));
    auto h4 = std::hash<T4>{}(std::get<3>(p));

    return h1 ^ h2 ^ h3 ^ h4; // use hash combine here
  }
};

struct BLPHeader;

struct scoped_blp_texture_reference;
struct blp_texture : public AsyncObject
{
  blp_texture (BlizzardArchive::Listfile::FileKey const& filename, Noggit::NoggitRenderContext context);
  void finishLoading() override;
  virtual void waitForChildrenLoaded() override {};

  void loadFromUncompressedData(BLPHeader const* lHeader, char const* lData);
  void loadFromCompressedData(BLPHeader const* lHeader, char const* lData);

  int width() const { return _width; }
  int height() const { return _height; }

  void bind();
  void upload();
  void uploadToArray(unsigned layer);
  void unload();
  bool is_uploaded() { return _uploaded; };
  GLuint texture_array() { return _texture_array; };
  int array_index() { return _array_index; };
  bool is_specular() { return _is_specular; };
  unsigned mip_level() { return !_compression_format ? _data.size() : _compressed_data.size(); };

  std::map<int, std::vector<uint32_t>>& data() { return _data;};
  std::map<int, std::vector<uint8_t>>& compressed_data() { return _compressed_data; };
  std::optional<GLint> const& compression_format() { return _compression_format; };

  Noggit::NoggitRenderContext getContext() { return _context; };

  [[nodiscard]]
  async_priority loading_priority() const override
  {
    return async_priority::high;
  }

private:
  bool _uploaded = false;

  int _width;
  int _height;

  Noggit::NoggitRenderContext _context;

  bool _is_specular = false;
  bool _is_tileset = false;

private:
  std::map<int, std::vector<uint32_t>> _data;
  std::map<int, std::vector<uint8_t>> _compressed_data;
  std::optional<GLint> _compression_format;
  int _array_index = -1;
  GLuint _texture_array = 0;
};

struct TexArrayParams
{
  std::vector<GLuint> arrays;
  int n_used = 0;
};

class TextureManager
{
public:
  static void report();
  static void unload_all(Noggit::NoggitRenderContext context);
  static TexArrayParams& get_tex_array(int width, int height, int mip_level, Noggit::NoggitRenderContext context);
  static TexArrayParams& get_tex_array(GLint compression, int width, int height, int mip_level, std::map<int, std::vector<uint8_t>>& comp_data, Noggit::NoggitRenderContext context);

private:
  friend struct scoped_blp_texture_reference;
  static Noggit::AsyncObjectMultimap<blp_texture> _;
  static std::array<std::unordered_map<std::tuple<GLint, int, int, int>, TexArrayParams, tuple_hash>, 7> _tex_arrays;

};

struct scoped_blp_texture_reference
{
  scoped_blp_texture_reference() = delete;
  scoped_blp_texture_reference (std::string const& filename, Noggit::NoggitRenderContext context);
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
  Noggit::NoggitRenderContext _context;
};

namespace Noggit
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
    std::unique_ptr<OpenGL::program> _program;

    OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
    OpenGL::Scoped::deferred_upload_buffers<3> _buffers;

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
