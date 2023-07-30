// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <opengl/scoped.hpp>
#include <opengl/context.hpp>
#include <opengl/context.inl>
#include <opengl/shader.hpp>
#include <opengl/texture.hpp>
#include <noggit/Misc.h>
#include <glm/vec3.hpp>

#include <QFile>
#include <QTextStream>

#include <stdexcept>
#include <list>
#include <regex>
#include <sstream>

namespace OpenGL
{
  shader::shader (GLenum type, std::string const& source)
  try
    : _handle (gl.createShader (type))
  {
    char const* source_ptr (source.data());
    gl.shaderSource (_handle, 1, &source_ptr, nullptr);
    gl.compile_shader (_handle);
  }
  catch (...)
  {
    std::throw_with_nested
      (std::runtime_error ("error constructing shader '" + source + "'"));
  }
  shader::~shader()
  {
    gl.deleteShader (_handle);
  }

  std::string shader::src_from_qrc(std::string const& shader_alias)
  {
    QFile f(QString::fromStdString(":/shader/" + shader_alias));

    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
      throw std::logic_error("Could not load " + shader_alias + " from the qrc file");
    }

    QTextStream stream(&f);

    return stream.readAll().toStdString();
  }

  std::string shader::src_from_qrc(std::string const& shader_alias, std::vector<std::string> const& defines)
  {
    std::string src(src_from_qrc(shader_alias));

    if (defines.empty())
    {
      return src;
    }

    std::stringstream ss;

    ss << "\n";
    for (auto const& def : defines)
    {
      ss << "#define " << def << "\n";
    }

    std::regex regex("#version[ \t]+[0-9]+.*");
    std::smatch match;

    if (!std::regex_search(src, match, regex))
    {
      throw std::logic_error("shader " + shader_alias + " has no #version directive");
    }

    // #version is always the first thing in the shader, insert defines after it
    src.insert(match.length() + match.position(), ss.str());

    return src;
  }

  program::program (std::initializer_list<shader> shaders)
    : _handle (gl.createProgram())
  {
    struct scoped_attach
    {
      scoped_attach (GLuint program, GLuint shader)
        : _program (program)
        , _shader (shader)
      {
        gl.attachShader (_program, _shader);
      }
      ~scoped_attach()
      {
        gl.detachShader (_program, _shader);
      }
      GLuint _program;
      GLuint _shader;
    };

    std::list<scoped_attach> attachments;

    for (shader const& s : shaders)
    {
      attachments.emplace_back (*_handle, s._handle);
    }

    gl.link_program (*_handle);
#ifdef  VALIDATE_OPENGL_PROGRAMS
    gl.validate_program(*_handle);
#endif
  }
  program::program (program&& other)
    : _handle (std::nullopt)
  {
    std::swap (_handle, other._handle);
  }
  program::~program()
  {
    if (_handle)
    {
      gl.deleteProgram (*_handle);
    }
  }

  GLuint program::uniform_location (std::string const& name) const
  {
    return gl.getUniformLocation (*_handle, name.c_str());
  }
  GLuint program::uniform_block_location (std::string const& name) const
  {
    return gl.getUniformBlockIndex(*_handle, name.c_str());
  }
  GLuint program::attrib_location (std::string const& name) const
  {
    return gl.getAttribLocation (*_handle, name.c_str());
  }

  namespace Scoped
  {
    use_program::use_program (program const& p)
      : _program (p)
    {
      gl.getIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*> (&_old));
      gl.useProgram (*_program._handle);
    }
    use_program::~use_program()
    {
      for (auto const& array : _enabled_vertex_attrib_arrays)
      {
        GLint cur_vao = 0;
        gl.getIntegerv(GL_VERTEX_ARRAY_BINDING, &cur_vao);
        if(cur_vao != array || !cur_vao)
          continue;

        gl.disableVertexAttribArray (array);
      }
      gl.useProgram (_old);
    }

    void use_program::uniform (std::string const& name, GLint value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform1i (loc, value);
    }
    void use_program::uniform (GLint pos, GLint value)
    {
      gl.uniform1i (pos, value);
    }
    void use_program::uniform (std::string const& name, GLfloat value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform1f (loc, value);
    }
    void use_program::uniform (GLint pos, GLfloat value)
    {
      gl.uniform1f (pos, value);
    }
    void use_program::uniform (std::string const& name, bool value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform1i (loc, static_cast<int>(value));
    }
    void use_program::uniform (GLint pos, bool value)
    {
      gl.uniform1i (pos, static_cast<int>(value));
    }
    void use_program::uniform_cached(std::string const& name, GLint value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      auto cache = _program.getUniformsIntCache();
      auto it  = cache->find(loc);
      if (it == cache->end() || it->second != value)
      {
        (*const_cast<tsl::robin_map<GLuint, GLint>*>(cache))[loc] = value;
        gl.uniform1i(loc, value);
      }
    }
    void use_program::uniform_cached(std::string const& name, GLfloat value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      auto cache = _program.getUniformsFloatCache();
      auto it  = cache->find(loc);
      if (it == cache->end() || !misc::float_equals(it->second, value))
      {
        (*const_cast<tsl::robin_map<GLuint, GLfloat>*>(cache))[loc] = value;
        gl.uniform1f(loc, value);
      }
    }
    void use_program::uniform_cached(std::string const& name, bool value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      auto cache = _program.getUniformsBoolCache();
      auto it  = cache->find(loc);
      if (it == cache->end() || it->second != value)
      {
        (*const_cast<tsl::robin_map<GLuint, bool>*>(cache))[loc] = value;
        gl.uniform1i(loc, static_cast<int>(value));
      }
    }

    void use_program::bind_uniform_block(std::string const& name, unsigned target)
    {
        gl.uniformBlockBinding(_program._handle.value() , uniform_block_location(name), target);
    }
    void use_program::uniform (std::string const& name, std::vector<int> const& value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform1iv (loc, static_cast<GLsizei>(value.size()), value.data());
    }
    void use_program::uniform (std::string const& name, int const* data, std::size_t size)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform1iv (loc, static_cast<GLsizei>(size), data);
    }
    void use_program::uniform (std::string const& name, glm::vec3 const* data, std::size_t size)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform3fv (loc, static_cast<GLsizei>(size), reinterpret_cast<const GLfloat*>(data));
    }
    void use_program::uniform (GLint pos, std::vector<int> const& value)
    {
      gl.uniform1iv (pos, static_cast<GLsizei>(value.size()), value.data());
    }
    void use_program::uniform (std::string const& name, std::vector<glm::vec3> const& value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform3fv (loc, static_cast<GLsizei>(value.size()), glm::value_ptr(value[0]));
    }
    void use_program::uniform(std::string const& name, std::vector<glm::vec4> const& value)
    {
        GLuint loc = uniform_location(name);
        if (loc < 0)
            return;

        gl.uniform4fv(loc, static_cast<GLsizei>(value.size()), glm::value_ptr(value[0]));
    }
    void use_program::uniform_chunk_textures (std::string const& name, std::array<std::array<std::array<int, 2>, 4>, 256> const& value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform2iv (loc, 256 * 4, reinterpret_cast<const GLint*>(value.data()));
    }
    void use_program::uniform (GLint pos, std::vector<glm::vec3> const& value)
    {
      gl.uniform3fv (pos, static_cast<GLsizei>(value.size()),glm::value_ptr(value[0]));
    }
    void use_program::uniform(GLint pos, std::vector<glm::vec4> const& value)
    {
        gl.uniform4fv(pos, static_cast<GLsizei>(value.size()), glm::value_ptr(value[0]));
    }
    void use_program::uniform (std::string const& name, glm::vec2 const& value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform2fv(loc, 1, glm::value_ptr(value));
    }
    void use_program::uniform (GLint pos, glm::vec2 const& value)
    {
      gl.uniform2fv (pos, 1, glm::value_ptr(value));
    }
    void use_program::uniform (std::string const& name, glm::vec3 const& value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform3fv (loc, 1, glm::value_ptr(value));
    }
    void use_program::uniform (GLint pos, glm::vec3 const& value)
    {
      gl.uniform3fv (pos, 1, glm::value_ptr(value));
    }
    void use_program::uniform (std::string const& name, glm::vec4 const& value)
    {
      GLuint loc = uniform_location (name);
      if (loc < 0)
        return;

      gl.uniform4fv (loc, 1, glm::value_ptr(value));
    }
    void use_program::uniform (GLint pos, glm::vec4 const& value)
    {
      gl.uniform4fv (pos, 1, glm::value_ptr(value));
    }

    void use_program::uniform(std::string const& name, glm::mat4x4 const& value)
    {
        GLuint loc = uniform_location(name);
        if (loc < 0)
            return;

        gl.uniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }
    void use_program::uniform(GLint pos, glm::mat4x4 const& value)
    {
        gl.uniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(value));
    }

    void use_program::sampler (std::string const& name, GLenum texture_slot, texture* tex)
    {
      uniform (name, GLint (texture_slot - GL_TEXTURE0));
      texture::set_active_texture (texture_slot - GL_TEXTURE0);
      tex->bind();
    }

    void use_program::attrib (std::string const& name, std::vector<float> const& data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribPointer (location, 1, GL_FLOAT, GL_FALSE, 0, data.data());
    }
    void use_program::attrib (std::string const& name, std::vector<glm::vec2> const& data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribPointer (location, 2, GL_FLOAT, GL_FALSE, 0, data.data());
    }
    void use_program::attrib (std::string const& name, std::vector<glm::vec3> const& data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribPointer (location, 3, GL_FLOAT, GL_FALSE, 0, data.data());
    }
    void use_program::attrib (std::string const& name, glm::vec3 const* data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribPointer (location, 3, GL_FLOAT, GL_FALSE, 0, data);
    }
    void use_program::attrib (std::string const& name, glm::mat4x4 const* data, GLuint divisor)
    {
      GLuint const location (attrib_location (name));
      glm::vec4 const* vec4_ptr = reinterpret_cast<glm::vec4 const*>(data);

      for (GLuint i = 0; i < 4; ++i)
      {
        gl.enableVertexAttribArray (location + i);
        _enabled_vertex_attrib_arrays.emplace (location + i);
        gl.vertexAttribPointer (location + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4x4), vec4_ptr + i);
        gl.vertexAttribDivisor(location + i, divisor);
      }      
    }
    void use_program::attrib (std::string const& name, GLsizei size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribPointer (location, size, type, normalized, stride, data);
    }
    void use_program::attrib (std::string const& name, GLuint buffer, GLsizei size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      Scoped::buffer_binder<GL_ARRAY_BUFFER> const bind (buffer);
      gl.vertexAttribPointer (location, size, type, normalized, stride, data);
    }

    void use_program::attribi (std::string const& name, GLsizei size, GLenum type, GLsizei stride, const GLvoid* data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      gl.vertexAttribIPointer (location, size, type, stride, data);
    }

    void use_program::attribi (std::string const& name, GLuint buffer, GLsizei size, GLenum type, GLsizei stride, const GLvoid* data)
    {
      GLuint const location (attrib_location (name));
      gl.enableVertexAttribArray (location);
      _enabled_vertex_attrib_arrays.emplace (location);
      Scoped::buffer_binder<GL_ARRAY_BUFFER> const bind (buffer);
      gl.vertexAttribIPointer (location, size, type, stride, data);
    }


    void use_program::attrib_divisor(std::string const& name, GLuint divisor, GLsizei range)
    {
      GLuint const location (attrib_location (name));
      for (GLuint i = 0; i < static_cast<GLuint>(range); ++i)
      {
        gl.vertexAttribDivisor(location + i, divisor);
      }
    }

    GLuint use_program::uniform_location (std::string const& name)
    {
      auto uniforms = _program.getUniforms();
      auto it  = uniforms->find(name);
      if (it != uniforms->end())
      {
        return it->second;
      }

      GLuint loc = _program.uniform_location(name);
      if (loc == -1)
      {
        LogError <<  "uniform " + name + " does not exist in shader\n" << std::endl;
      }
      (*const_cast<tsl::robin_map<std::string, GLuint>*>(uniforms))[name] = loc;
      return loc;
    }

    GLuint use_program::uniform_block_location (std::string const& name)
    {
      auto uniforms = _program.getUniforms();
      auto it  = uniforms->find(name);
      if (it != uniforms->end())
      {
        return it->second;
      }

      GLuint loc = _program.uniform_block_location(name);
      if (loc == -1)
      {
        throw std::invalid_argument ("uniform block " + name + " does not exist in shader\n");
      }
      (*const_cast<tsl::robin_map<std::string, GLuint>*>(uniforms))[name] = loc;
      return loc;
    }

    GLuint use_program::attrib_location (std::string const& name)
    {
      auto attribs = _program.getAttributes();
      auto it = attribs->find (name);
      if (it != attribs->end())
      {
        return it->second;
      }
      else
      {
        GLuint loc = _program.attrib_location (name);
        if (loc == -1)
        {
          throw std::invalid_argument ("attribute " + name + " does not exist in shader\n");
        }
        (*const_cast<tsl::robin_map<std::string, GLuint>*>(attribs))[name] = loc;
        return loc;
      }
    }
  }
}
