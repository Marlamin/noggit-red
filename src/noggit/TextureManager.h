// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncObject.h>
#include <noggit/ContextObject.hpp>
#include <noggit/AsyncObjectMultimap.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <QtGui/QPixmap>
#include <optional>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>
#include <tuple>

class QOffscreenSurface;
class QOpenGLFramebufferObjectFormat;

struct texture_heightmapping_data
{
    texture_heightmapping_data(uint32_t scale = 0, float heightscale = 0, float heightoffset = 1.0f)
    {
        uvScale = scale;
        heightScale = heightscale;
        heightOffset = heightoffset;
    }
    uint32_t uvScale = 0;
    float heightScale = 0.0f;
    float heightOffset = 1.0f;
};

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

struct blp_texture : public AsyncObject
{
  blp_texture (BlizzardArchive::Listfile::FileKey const& filename, Noggit::NoggitRenderContext context);
  void finishLoading() override;
  virtual void waitForChildrenLoaded() override {};

  void loadFromUncompressedData(BLPHeader const* lHeader, char const* lData);
  void loadFromCompressedData(BLPHeader const* lHeader, char const* lData);

  int width() const;
  int height() const;

  void bind();
  void upload();
  void uploadToArray(unsigned layer);
  void unload();
  bool is_uploaded() const;;
  GLuint texture_array() const;;
  int array_index() const;;
  bool is_specular() const;;
  unsigned mip_level() const;;

  std::map<int, std::vector<uint32_t>>& data();;
  std::map<int, std::vector<uint8_t>>& compressed_data();;
  std::optional<GLint> const& compression_format() const;;

  Noggit::NoggitRenderContext getContext() const;;

  [[nodiscard]]
  async_priority loading_priority() const override;
  // Mists HeightMapping
  bool hasHeightMap() const;;

  blp_texture* getHeightMap();;
private:
  bool _uploaded = false;

  int _width;
  int _height;

  Noggit::NoggitRenderContext _context;

  bool _is_specular = false;
  bool _is_tileset = false;
  bool _has_heightmap = false;

private:
  std::map<int, std::vector<uint32_t>> _data;
  std::map<int, std::vector<uint8_t>> _compressed_data;
  std::optional<GLint> _compression_format;
  int _array_index = -1;
  GLuint _texture_array = 0;

  std::unique_ptr<blp_texture> heightMap;
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
  static std::array<std::unordered_map<std::tuple<GLint, int, int, int>, TexArrayParams, tuple_hash>, Noggit::NoggitRenderContext::count> _tex_arrays;

};

namespace Noggit
{

  class BLPRenderer
  {
  private:
    BLPRenderer() = default;

    BLPRenderer( const BLPRenderer&) = delete;
    BLPRenderer& operator=( BLPRenderer& ) = delete;

    std::map<std::tuple<std::string, int, int>, QPixmap> _cache;

    std::unique_ptr<QOpenGLContext> _context;
    std::unique_ptr<QOpenGLFramebufferObjectFormat> _fmt;
    std::unique_ptr<QOffscreenSurface> _surface;
    std::unique_ptr<OpenGL::program> _program;

    OpenGL::Scoped::deferred_upload_vertex_arrays<1> _vao;
    OpenGL::Scoped::deferred_upload_buffers<3> _buffers;

    bool _uploaded = false;

  public:
    static BLPRenderer& getInstance();

    QPixmap* render_blp_to_pixmap ( std::string const& blp_filename, int width = -1, int height = -1);
    void unload();
    void upload();

  };


}
