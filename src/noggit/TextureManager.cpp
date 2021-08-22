// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/vector_2d.hpp>
#include <noggit/TextureManager.h>
#include <noggit/Log.h> // LogDebug

#include <QtCore/QString>
#include <QtGui/QPixmap>

#include <algorithm>

decltype (TextureManager::_) TextureManager::_;
decltype (TextureManager::_tex_arrays) TextureManager::_tex_arrays;

void TextureManager::report()
{
  std::string output = "Still in the Texture manager:\n";
  _.apply ( [&] (std::string const& key, blp_texture const&)
            {
              output += " - " + key + "\n";
            }
          );
  LogDebug << output;
}

void TextureManager::unload_all(noggit::NoggitRenderContext context)
{
  _.context_aware_apply(
      [&] (std::string const&, blp_texture& blp_texture)
      {
          blp_texture.unload();
      }
      , context
  );
}

TexArrayParams& TextureManager::get_tex_array(int width, int height, int mip_level,
                                              noggit::NoggitRenderContext context)
{
  TexArrayParams& array_params = _tex_arrays[context][std::make_tuple(-1, width, height, mip_level)];

  GLint n_layers = 0;
  gl.getIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &n_layers);

  int index_x = array_params.n_used / n_layers;

  if (array_params.arrays.size() <= index_x)
  {
    GLuint array;

    gl.genTextures(1, &array);
    gl.bindTexture(GL_TEXTURE_2D_ARRAY, array);

    array_params.arrays.emplace_back(array);

    GLint n_layers = 0;
    gl.getIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &n_layers);

    int width_ = width;
    int height_ = height;

    for (int i = 0; i < mip_level; ++i)
    {
      gl.texImage3D(GL_TEXTURE_2D_ARRAY, i, GL_RGBA8, width_, height_, n_layers, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                    nullptr);

      width_ = std::max(width_ >> 1, 1);
      height_ = std::max(height_ >> 1, 1);
    }

    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, mip_level - 1);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else
  {
    gl.bindTexture(GL_TEXTURE_2D_ARRAY, array_params.arrays[index_x]);
  }

  return array_params;
}

TexArrayParams& TextureManager::get_tex_array(GLint compression, int width, int height, int mip_level,
                              std::map<int, std::vector<uint8_t>>& comp_data, noggit::NoggitRenderContext context)
{

  TexArrayParams& array_params = _tex_arrays[context][std::make_tuple(compression, width, height, mip_level)];

  GLint n_layers = 0;
  gl.getIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &n_layers);

  int index_x = array_params.n_used / n_layers;

  if (array_params.arrays.size() <= index_x)
  {
    GLuint array;

    gl.genTextures(1, &array);
    gl.bindTexture(GL_TEXTURE_2D_ARRAY, array);

    array_params.arrays.emplace_back(array);

    int width_ = width;
    int height_ = height;

    for (int i = 0; i < mip_level; ++i)
    {
      gl.compressedTexImage3D(GL_TEXTURE_2D_ARRAY, i, compression, width_, height_, n_layers, 0, comp_data[i].size() * n_layers, nullptr);

      width_ = std::max(width_ >> 1, 1);
      height_ = std::max(height_ >> 1, 1);
    }

    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, mip_level - 1);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else
  {
    gl.bindTexture(GL_TEXTURE_2D_ARRAY, array_params.arrays[index_x]);
  }

  return array_params;
}

#include <cstdint>
//! \todo Cross-platform syntax for packed structs.
#pragma pack(push,1)
struct BLPHeader
{
  int32_t magix;
  int32_t version;
  uint8_t attr_0_compression;
  uint8_t attr_1_alphadepth;
  uint8_t attr_2_alphatype;
  uint8_t attr_3_mipmaplevels;
  int32_t resx;
  int32_t resy;
  int32_t offsets[16];
  int32_t sizes[16];
};
#pragma pack(pop)

#include <boost/thread.hpp>
#include <noggit/MPQ.h>

void blp_texture::bind()
{
  opengl::texture::bind();

  if (!finished)
  {
    return;
  }

  if (!_uploaded)
  {
    upload();
  }

  gl.bindTexture(GL_TEXTURE_2D_ARRAY, _texture_array);
}

void blp_texture::upload()
{
  if (_uploaded)
  {
    return;
  }

  int width = _width, height = _height;

  GLint n_layers;
  gl.getIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &n_layers);

  if (!_compression_format)
  {
    auto& params = TextureManager::get_tex_array( _width, _height, _data.size(), _context);

    int index_x = params.n_used / n_layers;
    int index_y = params.n_used % n_layers;

    _texture_array = params.arrays[index_x];
    _array_index = index_y;

    for (int i = 0; i < _data.size(); ++i)
    {
      gl.texImage2D(GL_TEXTURE_2D, i, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data[i].data());
      gl.texSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, index_y, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, _data[i].data());

      width = std::max(width >> 1, 1);
      height = std::max(height >> 1, 1);
    }

    params.n_used++;

    //LogDebug << "Mip level: " << std::to_string(_data.size()) << std::endl;

    _data.clear();
  }
  else
  {
    auto& params = TextureManager::get_tex_array(_compression_format.get(), _width, _height, _compressed_data.size(), _compressed_data, _context);

    int index_x = params.n_used / n_layers;
    int index_y = params.n_used % n_layers;

    _texture_array = params.arrays[index_x];
    _array_index = index_y;

    for (int i = 0; i < _compressed_data.size(); ++i)
    {
      gl.compressedTexImage2D(GL_TEXTURE_2D, i, _compression_format.get(), width, height, 0, _compressed_data[i].size(), _compressed_data[i].data());
      gl.compressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, index_y, width, height, 1, _compression_format.get(), _compressed_data[i].size(), _compressed_data[i].data());

      width = std::max(width >> 1, 1);
      height = std::max(height >> 1, 1);
    }

    params.n_used++;

    //LogDebug << "Mip level (compressed): " << std::to_string(_compressed_data.size()) << std::endl;

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, _compressed_data.size() - 1);
    _compressed_data.clear();
  }

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  _uploaded = true;
}

void blp_texture::unload()
{
  _uploaded = false;
  opengl::texture::unload();

  // load data back from file. pretty sad. maybe keep it after loading?
  finishLoading();
}

void blp_texture::loadFromUncompressedData(BLPHeader const* lHeader, char const* lData)
{
  unsigned int const* pal = reinterpret_cast<unsigned int const*>(lData + sizeof(BLPHeader));

  unsigned char const* buf;
  unsigned int *p;
  unsigned char const* c;
  unsigned char const* a;

  int alphabits = lHeader->attr_1_alphadepth;
  bool hasalpha = alphabits != 0;

  int width = _width, height = _height;

  for (int i = 0; i<16; ++i)
  {
    width = std::max(1, width);
    height = std::max(1, height);

    if (lHeader->offsets[i] > 0 && lHeader->sizes[i] > 0)
    {
      buf = reinterpret_cast<unsigned char const*>(&lData[lHeader->offsets[i]]);

      std::vector<uint32_t> data(lHeader->sizes[i]);

      int cnt = 0;
      p = data.data();
      c = buf;
      a = buf + width*height;
      for (int y = 0; y<height; y++)
      {
        for (int x = 0; x<width; x++)
        {
          unsigned int k = pal[*c++];
          k = ((k & 0x00FF0000) >> 16) | ((k & 0x0000FF00)) | ((k & 0x000000FF) << 16);
          int alpha = 0xFF;
          if (hasalpha)
          {
            if (alphabits == 8)
            {
              alpha = (*a++);
            }
            else if (alphabits == 1)
            {
              alpha = (*a & (1 << cnt++)) ? 0xff : 0;
              if (cnt == 8)
              {
                cnt = 0;
                a++;
              }
            }
          }

          k |= alpha << 24;
          *p++ = k;
        }
      }

      _data[i] = data;
    }
    else
    {
      return;
    }

    width >>= 1;
    height >>= 1;
  }
}

void blp_texture::loadFromCompressedData(BLPHeader const* lHeader, char const* lData)
{
  //                         0 (0000) & 3 == 0                1 (0001) & 3 == 1                    7 (0111) & 3 == 3
  const int alphatypes[] = { GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT };
  const int blocksizes[] = { 8, 16, 0, 16 };

  int alpha_type = lHeader->attr_2_alphatype & 3;
  GLint format = alphatypes[alpha_type];
  _compression_format = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? (lHeader->attr_1_alphadepth == 1 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT) : format;

  int width = _width, height = _height;

  for (int i = 0; i < 16; ++i)
  {
    if (lHeader->sizes[i] <= 0 || lHeader->offsets[i] <= 0)
    {
      return;
    }

    // make sure the vector is of the right size, blizzard seems to fuck those up for some small mipmaps
    int size = std::floor((width + 3) / 4) * std::floor((height + 3) / 4) * blocksizes[alpha_type];

    if (size < lHeader->sizes[i])
    {
      LogDebug << "mipmap size mismatch in '" << filename << "'" << std::endl;
      return;
    }

    _compressed_data[i].resize(size);

    char const* start = lData + lHeader->offsets[i];
    std::copy(start, start + lHeader->sizes[i], _compressed_data[i].begin());

    width = std::max(width >> 1, 1);
    height = std::max(height >> 1, 1);
  }
}

blp_texture::blp_texture(const std::string& filenameArg, noggit::NoggitRenderContext context)
  : AsyncObject(filenameArg)
  , _context(context)
{
}

void blp_texture::finishLoading()
{
  bool exists = MPQFile::exists(filename);
  if (!exists)
  {
    LogError << "file not found: '" << filename << "'" << std::endl;
  }

  MPQFile f(exists ? filename : "textures/shanecube.blp");
  if (f.isEof())
  {
    finished = true;
    throw std::runtime_error ("File " + filename + " does not exist");
  }

  char const* lData = f.getPointer();
  BLPHeader const* lHeader = reinterpret_cast<BLPHeader const*>(lData);
  _width = lHeader->resx;
  _height = lHeader->resy;

  if (lHeader->attr_0_compression == 1)
  {
    loadFromUncompressedData(lHeader, lData);
  }
  else if (lHeader->attr_0_compression == 2)
  {
    loadFromCompressedData(lHeader, lData);
  }
  else
  {
    finished = true;
    throw std::logic_error ("unimplemented BLP colorEncoding");

  }

  f.close();
  finished = true;
  _state_changed.notify_all();
}

namespace noggit
{

  BLPRenderer::BLPRenderer()
  {
    _cache = {};

    opengl::context::save_current_context const context_save (::gl);
    _context.create();

    _fmt.setSamples(1);
    _fmt.setInternalTextureFormat(GL_RGBA8);

    _surface.create();

    _context.makeCurrent(&_surface);

    opengl::context::scoped_setter const context_set (::gl, &_context);

    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> cull;
    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth;

    _vao.upload();
    _buffers.upload();

    GLuint const& indices_vbo = _buffers[0];
    GLuint const& vertices_vbo = _buffers[1];
    GLuint const& texcoords_vbo = _buffers[2];

    std::vector<math::vector_2d> vertices =
        {
             {-1.0f, -1.0f}
            ,{-1.0f, 1.0f}
            ,{ 1.0f, 1.0f}
            ,{ 1.0f, -1.0f}
        };
    std::vector<math::vector_2d> texcoords =
        {
             {0.f, 0.f}
            ,{0.f, 1.0f}
            ,{1.0f, 1.0f}
            ,{1.0f, 0.f}
        };
    std::vector<std::uint16_t> indices = {0,1,2, 2,3,0};

    gl.bufferData<GL_ARRAY_BUFFER, math::vector_2d>(vertices_vbo, vertices, GL_STATIC_DRAW);
    gl.bufferData<GL_ARRAY_BUFFER, math::vector_2d>(texcoords_vbo, texcoords, GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(indices_vbo, indices, GL_STATIC_DRAW);


    _program.reset(new opengl::program
                      (
                          {
                              {
                                  GL_VERTEX_SHADER, R"code(
                                  #version 330 core

                                  in vec4 position;
                                  in vec2 tex_coord;
                                  out vec2 f_tex_coord;

                                  uniform float width;
                                  uniform float height;

                                  void main()
                                  {
                                    f_tex_coord = vec2(tex_coord.x * width, -tex_coord.y * height);
                                    gl_Position = vec4(position.x * width / 2, position.y * height / 2, position.z, 1.0);
                                  }
                                  )code"
                              },
                              {
                                  GL_FRAGMENT_SHADER, R"code(
                                  #version 330 core

                                  uniform sampler2D tex;

                                  in vec2 f_tex_coord;

                                  layout(location = 0) out vec4 out_color;

                                  void main()
                                  {
                                    out_color = vec4(texture(tex, f_tex_coord/2.f + vec2(0.5)).rgb, 1.);
                                  }
                                  )code"
                              }
                          }
                      ));
    
    opengl::scoped::use_program shader (*_program.get());
    
    opengl::scoped::vao_binder const _ (_vao[0]);

    {
      opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> vertices_binder (vertices_vbo);
      shader.attrib("position", 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    {
      opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> texcoords_binder (texcoords_vbo);
      shader.attrib("tex_coord", 2, GL_FLOAT, GL_FALSE, 0, 0);
    }


  }

  QPixmap* BLPRenderer::render_blp_to_pixmap ( std::string const& blp_filename
                                               , int width
                                               , int height
                                               )
  {
    std::tuple<std::string, int, int> const curEntry{blp_filename, width, height};
    auto it{_cache.find(curEntry)};

    if(it != _cache.end())
      return &it->second;

    opengl::context::save_current_context const context_save (::gl);

    _context.makeCurrent(&_surface);

    opengl::context::scoped_setter const context_set (::gl, &_context);

    opengl::texture::set_active_texture(0);
    blp_texture texture(blp_filename, noggit::NoggitRenderContext::BLP_RENDERER);
    texture.finishLoading();

    width = width == -1 ? texture.width() : width;
    height = height == -1 ? texture.height() : height;

    float h = static_cast<float>(height);
    float w = static_cast<float>(width);

    QOpenGLFramebufferObject pixel_buffer(width, height, _fmt);
    pixel_buffer.bind();

    gl.viewport(0, 0, w, h);
    gl.clearColor(.0f, .0f, .0f, 1.f);
    gl.clear(GL_COLOR_BUFFER_BIT);
    
    opengl::scoped::use_program shader (*_program.get());

    shader.uniform("tex", 0);
    shader.uniform("width", w);
    shader.uniform("height", h);

    texture.bind();

    opengl::scoped::vao_binder const _ (_vao[0]);
    
    opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> indices_binder(_buffers[0]);

    gl.drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    QPixmap result{};
    result = std::move(QPixmap::fromImage(pixel_buffer.toImage()));
    pixel_buffer.release();

    if (result.isNull())
    {
      throw std::runtime_error
        ("failed rendering " + blp_filename + " to pixmap");
    }

    return &(_cache[curEntry] = std::move(result));
  }

  BLPRenderer::~BLPRenderer()
  {
    opengl::context::scoped_setter const context_set (::gl, &_context);
  }
}

scoped_blp_texture_reference::scoped_blp_texture_reference (std::string const& filename, noggit::NoggitRenderContext context)
  : _blp_texture(TextureManager::_.emplace(filename, context))
  , _context(context)
{}

scoped_blp_texture_reference::scoped_blp_texture_reference (scoped_blp_texture_reference const& other)
  : _blp_texture(other._blp_texture ? TextureManager::_.emplace(other._blp_texture->filename, other._context) : nullptr)
  , _context(other._context)
{}

void scoped_blp_texture_reference::Deleter::operator() (blp_texture* texture) const
{
  TextureManager::_.erase(texture->filename, texture->getContext());
}

blp_texture* scoped_blp_texture_reference::operator->() const
{
  return _blp_texture.get();
}
blp_texture* scoped_blp_texture_reference::get() const
{
  return _blp_texture.get();
}

bool scoped_blp_texture_reference::operator== (scoped_blp_texture_reference const& other) const
{
  return std::tie(_blp_texture) == std::tie(other._blp_texture);
}
