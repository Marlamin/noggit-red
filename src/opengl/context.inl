// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifndef NOGGIT_CONTEXT_INL
#define NOGGIT_CONTEXT_INL

#include <opengl/context.hpp>
#include <noggit/Log.h>
#include <math/vector_2d.hpp>
#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>

#include <QtOpenGLExtensions/QOpenGLExtensions>
#include <QtGui/QOpenGLFunctions>
#include <boost/current_function.hpp>


namespace
{
  std::size_t inside_gl_begin_end = 0;

  template<typename Extension> struct extension_traits;
  template<> struct extension_traits<QOpenGLExtension_ARB_vertex_program>
  {
    static constexpr char const* const name = "GL_ARB_vertex_program";
  };

  struct verify_context_and_check_for_gl_errors
  {
    template<typename ExtraInfo> verify_context_and_check_for_gl_errors
      (QOpenGLContext* current_context, char const* function, ExtraInfo&& extra_info)
      : _current_context (current_context)
      , _function (function)
      , _extra_info (extra_info)
    {
      if (!_current_context)
      {
        throw std::runtime_error (std::string(_function) + ": called without active OpenGL context: no context at all");
      }
      if (!_current_context->isValid())
      {
        throw std::runtime_error (std::string(_function) + ": called without active OpenGL context: invalid");
      }
      if (QOpenGLContext::currentContext() != _current_context)
      {
        throw std::runtime_error (std::string(_function) + ": called without active OpenGL context: not current context");
      }
    }
    verify_context_and_check_for_gl_errors (QOpenGLContext* current_context, char const* function)
      : verify_context_and_check_for_gl_errors (current_context, function, &verify_context_and_check_for_gl_errors::no_extra_info)
    {}

    template<typename Functions>
    Functions* version_functions() const
    {
      Functions* f (_current_context->versionFunctions<Functions>());
      if (!f)
      {
        throw std::runtime_error (std::string(_function) + ": requires OpenGL functions for version " + typeid (Functions).name());
      }
      return f;
    }
    template<typename Extension>
    std::unique_ptr<Extension> extension_functions() const
    {
      if (!_current_context->hasExtension (extension_traits<Extension>::name))
      {
        throw std::runtime_error (std::string(_function) + ": requires OpenGL extension " + extension_traits<Extension>::name);
      }
      std::unique_ptr<Extension> functions (new Extension());
      functions->initializeOpenGLFunctions();
      return functions;
    }

    QOpenGLContext* _current_context;
    char const* _function;
    static std::string no_extra_info() { return {}; }
    std::function<std::string()> _extra_info;

    ~verify_context_and_check_for_gl_errors()
    {
      if (inside_gl_begin_end)
      {
        return;
      }

      std::string errors;
      std::size_t error_count = 0;
      while (GLenum error = glGetError())
      {
        if (error_count >= 10)
          break;

        switch (error)
        {
          case GL_INVALID_ENUM: errors += " GL_INVALID_ENUM"; break;
          case GL_INVALID_FRAMEBUFFER_OPERATION: errors += " GL_INVALID_FRAMEBUFFER_OPERATION"; break;
          case GL_INVALID_OPERATION: errors += " GL_INVALID_OPERATION"; break;
          case GL_INVALID_VALUE: errors += " GL_INVALID_VALUE"; break;
          case GL_OUT_OF_MEMORY: errors += " GL_OUT_OF_MEMORY"; break;
          case GL_STACK_OVERFLOW: errors += " GL_STACK_OVERFLOW"; break;
          case GL_STACK_UNDERFLOW: errors += " GL_STACK_UNDERFLOW"; break;
          case GL_TABLE_TOO_LARGE: errors += " GL_TABLE_TOO_LARGE"; break;
          default: errors += " UNKNOWN_ERROR (" + std::to_string(error) + ")"; break;
        }

        ++error_count;
      }

      if (error_count == 10 && glGetError())
      {
        errors += " and more...";
      }

      if (!errors.empty())
      {
        errors += _extra_info();
#ifndef NOGGIT_DO_NOT_THROW_ON_OPENGL_ERRORS
        LogError << _function << ":" + errors << std::endl;
#else
        throw std::runtime_error (_function + ":" + errors);
#endif
      }
    }

    verify_context_and_check_for_gl_errors (verify_context_and_check_for_gl_errors const&) = delete;
    verify_context_and_check_for_gl_errors (verify_context_and_check_for_gl_errors&&) = delete;
    verify_context_and_check_for_gl_errors& operator= (verify_context_and_check_for_gl_errors const&) = delete;
    verify_context_and_check_for_gl_errors& operator= (verify_context_and_check_for_gl_errors&&) = delete;
  };
}

void opengl::context::enable (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glEnable (target);
}
void opengl::context::disable (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDisable (target);
}
GLboolean opengl::context::isEnabled (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glIsEnabled (target);
}
void opengl::context::viewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glViewport (x, y, width, height);
}
void opengl::context::depthFunc (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDepthFunc (target);
}
void opengl::context::depthMask (GLboolean mask)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDepthMask (mask);
}
void opengl::context::blendFunc (GLenum sfactor, GLenum dfactor)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glBlendFunc (sfactor, dfactor);
}

void opengl::context::clear (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glClear (target);
}
void opengl::context::clearColor (GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glClearColor (r, g, b, a);
}

void opengl::context::readBuffer (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glReadBuffer (target);
}
void opengl::context::readPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glReadPixels (x, y, width, height, format, type, data);
}

void opengl::context::lineWidth (GLfloat width)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glLineWidth (width);
}

void opengl::context::pointParameterf (GLenum pname, GLfloat param)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glPointParameterf (pname, param);
}
void opengl::context::pointParameteri (GLenum pname, GLint param)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glPointParameteri (pname, param);
}
void opengl::context::pointParameterfv (GLenum pname, GLfloat const* param)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glPointParameterfv (pname, param);
}
void opengl::context::pointParameteriv (GLenum pname, GLint const* param)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glPointParameteriv (pname, param);
}
void opengl::context::pointSize (GLfloat size)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glPointSize (size);
}

void opengl::context::hint (GLenum target, GLenum mode)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glHint (target, mode);
}
void opengl::context::polygonMode (GLenum face, GLenum mode)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glPolygonMode (face, mode);
}

void opengl::context::genTextures (GLuint count, GLuint* textures)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGenTextures (count, textures);
}
void opengl::context::deleteTextures (GLuint count, GLuint* textures)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDeleteTextures (count, textures);
}
void opengl::context::bindTexture (GLenum target, GLuint texture)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glBindTexture (target, texture);
}
void opengl::context::texImage2D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid const* data)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glTexImage2D (target, level, internal_format, width, height, border, format, type, data);
}
void opengl::context::texSubImage2D(GLenum target,
                                    GLint level,
                                    GLint xoffset,
                                    GLint yoffset,
                                    GLsizei width,
                                    GLsizei height,
                                    GLenum format,
                                    GLenum type,
                                    const void * pixels)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
void opengl::context::texImage3D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLvoid const* data)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glTexImage3D (target, level, internal_format, width, height, depth, border, format, type, data);
}
void opengl::context::texSubImage3D (GLenum target,
                                    GLint level,
                                    GLint xoffset,
                                    GLint yoffset,
                                    GLint zoffset,
                                    GLsizei width,
                                    GLsizei height,
                                    GLsizei depth,
                                    GLenum format,
                                    GLenum type,
                                    const void * pixels)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glTexSubImage3D (target, level, xoffset,yoffset ,zoffset, width, height, depth, format, type, pixels);
}
void opengl::context::compressedTexSubImage3D (GLenum target,
                                                  GLint level,
                                                  GLint xoffset,
                                                  GLint yoffset,
                                                  GLint zoffset,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  GLsizei depth,
                                                  GLenum format,
                                                  GLsizei imageSize,
                                                  const void * data)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glCompressedTexSubImage3D (target, level, xoffset,yoffset ,zoffset, width, height, depth, format, imageSize, data);
}
void opengl::context::compressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid const* data)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glCompressedTexImage2D (target, level, internalformat, width, height, border, imageSize, data);
}
void opengl::context::compressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, GLvoid const* data)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glCompressedTexImage3D (target, level, internalformat, width, height, depth, border, imageSize, data);
}
void opengl::context::generateMipmap (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGenerateMipmap (target);
}
void opengl::context::activeTexture (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glActiveTexture (target);
}

void opengl::context::texParameteri (GLenum target, GLenum pname, GLint param)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glTexParameteri (target, pname, param);
}
void opengl::context::texParameterf (GLenum target, GLenum pname, GLfloat param)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glTexParameterf (target, pname, param);
}
void opengl::context::texParameteriv (GLenum target, GLenum pname, GLint const* params)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glTexParameteriv (target, pname, params);
}
void opengl::context::texParameterfv (GLenum target, GLenum pname, GLfloat const* params)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glTexParameterfv (target, pname, params);
}

void opengl::context::genVertexArrays (GLuint count, GLuint* arrays)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glGenVertexArrays(count, arrays);
}
void opengl::context::deleteVertexArray (GLuint count, GLuint* arrays)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glDeleteVertexArrays(count, arrays);
}
void opengl::context::bindVertexArray (GLenum array)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glBindVertexArray(array);
}
void opengl::context::genBuffers (GLuint count, GLuint* buffers)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGenBuffers (count, buffers);
}
void opengl::context::deleteBuffers (GLuint count, GLuint* buffers)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDeleteBuffers (count, buffers);
}
void opengl::context::bindBuffer (GLenum target, GLuint buffer)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glBindBuffer (target, buffer);
}
void opengl::context::bindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glBindBufferRange (target, index, buffer, offset, size);
}
GLvoid* opengl::context::mapBuffer (GLenum target, GLenum access)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glMapBuffer (target, access);
}
GLboolean opengl::context::unmapBuffer (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glUnmapBuffer (target);
}
void opengl::context::drawElements (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDrawElements (mode, count, type, indices);
}
void opengl::context::drawElementsInstanced (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices, GLsizei instancecount)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glDrawElementsInstanced (mode, count, type, indices, instancecount);
}
void opengl::context::drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid const* indices)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glDrawRangeElements (mode, start, end, count, type, indices);
}

void opengl::context::genPrograms (GLsizei count, GLuint* programs)
{
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
  return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glGenProgramsARB (count, programs);
}
void opengl::context::deletePrograms (GLsizei count, GLuint* programs)
{
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
  return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glDeleteProgramsARB (count, programs);
}
void opengl::context::bindProgram (GLenum target, GLuint program)
{
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
  return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glBindProgramARB (target, program);
}
void opengl::context::programString (GLenum target, GLenum format, GLsizei len, GLvoid const* pointer)
{
  verify_context_and_check_for_gl_errors const _
    ( _current_context
      , BOOST_CURRENT_FUNCTION
      , [this]
      {
        GLint error_position;
        getIntegerv (GL_PROGRAM_ERROR_POSITION_ARB, &error_position);
        return " at " + std::to_string (error_position) + ": " + reinterpret_cast<char const*> (getString (GL_PROGRAM_ERROR_STRING_ARB));
      }
    );
  return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glProgramStringARB (target, format, len, pointer);
}
void opengl::context::getProgramiv (GLuint program, GLenum pname, GLint* params)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGetProgramiv (program, pname, params);
}
void opengl::context::programLocalParameter4f (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
  return _.extension_functions<QOpenGLExtension_ARB_vertex_program>()->glProgramLocalParameter4fARB (target, index, x, y, z, w);
}

void opengl::context::getBooleanv (GLenum target, GLboolean* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGetBooleanv (target, value);
}
void opengl::context::getDoublev (GLenum target, GLdouble* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glGetDoublev (target, value);
}
void opengl::context::getFloatv (GLenum target, GLfloat* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGetFloatv (target, value);
}
void opengl::context::getIntegerv (GLenum target, GLint* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGetIntegerv (target, value);
}

GLubyte const* opengl::context::getString (GLenum target)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGetString (target);
}

GLuint opengl::context::createShader (GLenum shader_type)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glCreateShader (shader_type);
}
void opengl::context::deleteShader (GLuint shader)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDeleteShader (shader);
}
void opengl::context::shaderSource (GLuint shader, GLsizei count, GLchar const** string, GLint const* length)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glShaderSource (shader, count, string, length);
}
void opengl::context::compile_shader (GLuint shader)
{
  {
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
    _current_context->functions()->glCompileShader (shader);
  }
  if (get_shader (shader, GL_COMPILE_STATUS) != GL_TRUE)
  {
    std::vector<char> log (get_shader (shader, GL_INFO_LOG_LENGTH));
    _current_context->functions()->glGetShaderInfoLog (shader, log.size(), nullptr, log.data());
    LogDebug << std::string (log.data ()) << std::endl;
    throw std::runtime_error ("compiling shader failed: " + std::string (log.data()));
  }
}
GLint opengl::context::get_shader (GLuint shader, GLenum pname)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  GLint params;
  _current_context->functions()->glGetShaderiv (shader, pname, &params);
  return params;
}

GLuint opengl::context::createProgram()
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glCreateProgram();
}
void opengl::context::deleteProgram (GLuint program)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDeleteProgram (program);
}
void opengl::context::attachShader (GLuint program, GLuint shader)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glAttachShader (program, shader);
}
void opengl::context::detachShader (GLuint program, GLuint shader)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDetachShader (program, shader);
}
void opengl::context::link_program (GLuint program)
{
  {
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
    _current_context->functions()->glLinkProgram (program);
  }
  if (get_program (program, GL_LINK_STATUS) != GL_TRUE)
  {
    std::string error = get_program_info_log(program);
    LogError << "linking program failed: " << error << std::endl;
    throw std::runtime_error ("linking program failed: " + error);
  }
}
void opengl::context::useProgram (GLuint program)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glUseProgram (program);
}
void opengl::context::validate_program (GLuint program)
{
  // "The issue is that Mac does not allow validating shaders before
  // they are bound to a VBO. So, validating needs to be done after
  // that, not just after compiling the shader program. The doc says
  // that this is the way to be done afaik, but Windows/Linux does
  // not seem to care."
  if (QSysInfo::productType() == "osx")
  {
    return;
  }

  {
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
    verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
    _current_context->functions()->glValidateProgram (program);
  }
  if (get_program (program, GL_VALIDATE_STATUS) != GL_TRUE)
  {
    std::string error = get_program_info_log(program);
    LogError << "validating program failed: " << error << std::endl;
    throw std::runtime_error("validating program failed: " + error);
  }
}
GLint opengl::context::get_program (GLuint program, GLenum pname)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  GLint params;
  _current_context->functions()->glGetProgramiv (program, pname, &params);
  return params;
}
std::string opengl::context::get_program_info_log(GLuint program)
{
  verify_context_and_check_for_gl_errors const _(_current_context, BOOST_CURRENT_FUNCTION);
  std::vector<char> log(get_program(program, GL_INFO_LOG_LENGTH));

  if (log.empty())
  {
    return "<empty log>";
  }

  _current_context->functions()->glGetProgramInfoLog(program, log.size(), nullptr, log.data());

  return std::string(log.data());
}

GLint opengl::context::getAttribLocation (GLuint program, GLchar const* name)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGetAttribLocation (program, name);
}
void opengl::context::vertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid const* pointer)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glVertexAttribPointer (index, size, type, normalized, stride, pointer);
}
void opengl::context::vertexAttribDivisor (GLuint index, GLuint divisor)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glVertexAttribDivisor(index, divisor);
}
void opengl::context::enableVertexAttribArray (GLuint index)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glEnableVertexAttribArray (index);
}
void opengl::context::disableVertexAttribArray (GLuint index)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glDisableVertexAttribArray (index);
}

GLint opengl::context::getUniformLocation (GLuint program, GLchar const* name)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return (_current_context->functions()->glGetUniformLocation (program, name));
}

GLint opengl::context::getUniformBlockIndex (GLuint program, GLchar const* name)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif

  auto val (_4_1_core_func->glGetUniformBlockIndex(program, name));
  if (val == -1)
  {
    throw std::logic_error ("unknown uniform block " + std::string (name));
  }
  return val;
}

void opengl::context::uniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _4_1_core_func->glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

void opengl::context::uniform1i (GLint location, GLint value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glUniform1i (location, value);
}
void opengl::context::uniform1f (GLint location, GLfloat value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glUniform1f (location, value);
}

void opengl::context::uniform1iv (GLint location, GLsizei count, GLint const* value)
{
  verify_context_and_check_for_gl_errors const _(_current_context, BOOST_CURRENT_FUNCTION);
  return _current_context->functions()->glUniform1iv(location, count, value);
}

void opengl::context::uniform2iv (GLint location, GLsizei count, GLint const* value)
{
  verify_context_and_check_for_gl_errors const _(_current_context, BOOST_CURRENT_FUNCTION);
  return _current_context->functions()->glUniform2iv(location, count, value);
}

void opengl::context::uniform2fv (GLint location, GLsizei count, GLfloat const* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glUniform2fv (location, count, value);
}
void opengl::context::uniform3fv (GLint location, GLsizei count, GLfloat const* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glUniform3fv (location, count, value);
}
void opengl::context::uniform4fv (GLint location, GLsizei count, GLfloat const* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glUniform4fv (location, count, value);
}
void opengl::context::uniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, GLfloat const* value)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glUniformMatrix4fv (location, count, transpose, value);
}

void opengl::context::clearStencil (GLint s)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glClearStencil (s);
}
void opengl::context::stencilFunc (GLenum func, GLint ref, GLuint mask)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glStencilFunc (func, ref, mask);
}
void opengl::context::stencilOp (GLenum sfail, GLenum dpfail, GLenum dppass)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glStencilOp (sfail, dpfail, dppass);
}
void opengl::context::colorMask (GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glColorMask (r, g, b, a);
}

void opengl::context::polygonOffset (GLfloat factor, GLfloat units)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glPolygonOffset (factor, units);
}

void opengl::context::genFramebuffers (GLsizei n, GLuint *ids)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGenFramebuffers (n, ids);
}
void opengl::context::bindFramebuffer (GLenum target, GLuint framebuffer)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glBindFramebuffer (target, framebuffer);
}
void opengl::context::framebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glFramebufferTexture2D (target, attachment, textarget, texture, level);
}

void opengl::context::genRenderbuffers (GLsizei n, GLuint *ids)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glGenRenderbuffers (n, ids);
}
void opengl::context::bindRenderbuffer (GLenum target, GLuint renderbuffer)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glBindRenderbuffer (target, renderbuffer);
}
void opengl::context::renderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glRenderbufferStorage (target, internalformat, width, height);
}
void opengl::context::framebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glFramebufferRenderbuffer (target, attachment, renderbuffertarget, renderbuffer);
}

template<GLenum target>
void opengl::context::bufferData (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage)
{
  GLuint old;

#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  gl.getIntegerv ( target == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING
  : target == GL_DRAW_INDIRECT_BUFFER ? GL_DRAW_INDIRECT_BUFFER_BINDING
  : target == GL_ELEMENT_ARRAY_BUFFER ? GL_ELEMENT_ARRAY_BUFFER_BINDING
  : target == GL_PIXEL_PACK_BUFFER ? GL_PIXEL_PACK_BUFFER_BINDING
  : target == GL_PIXEL_UNPACK_BUFFER ? GL_PIXEL_UNPACK_BUFFER_BINDING
  : target == GL_TRANSFORM_FEEDBACK_BUFFER ? GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
  : target == GL_UNIFORM_BUFFER ? GL_UNIFORM_BUFFER_BINDING
  : throw std::logic_error ("bad bind target")
    , reinterpret_cast<GLint*> (&old)
  );
#endif

  gl.bindBuffer (target, buffer);
  bufferData (target, size, data, usage);
  gl.bindBuffer (target, old);
}
template void opengl::context::bufferData<GL_ARRAY_BUFFER> (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);
template void opengl::context::bufferData<GL_PIXEL_PACK_BUFFER> (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);
template void opengl::context::bufferData<GL_ELEMENT_ARRAY_BUFFER> (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);

template<GLenum target, typename T>
void opengl::context::bufferData(GLuint buffer, std::vector<T> const& data, GLenum usage)
{
  GLuint old;

#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  gl.getIntegerv ( target == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING
  : target == GL_DRAW_INDIRECT_BUFFER ? GL_DRAW_INDIRECT_BUFFER_BINDING
  : target == GL_ELEMENT_ARRAY_BUFFER ? GL_ELEMENT_ARRAY_BUFFER_BINDING
  : target == GL_PIXEL_PACK_BUFFER ? GL_PIXEL_PACK_BUFFER_BINDING
  : target == GL_PIXEL_UNPACK_BUFFER ? GL_PIXEL_UNPACK_BUFFER_BINDING
  : target == GL_TRANSFORM_FEEDBACK_BUFFER ? GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
  : target == GL_UNIFORM_BUFFER ? GL_UNIFORM_BUFFER_BINDING
  : throw std::logic_error ("bad bind target")
    , reinterpret_cast<GLint*> (&old)
  );
#endif

  gl.bindBuffer (target, buffer);
  bufferData(target, sizeof(T) * data.size(), data.data(), usage);
  gl.bindBuffer (target, old);
}

void opengl::context::bufferData (GLenum target, GLsizeiptr size, GLvoid const* data, GLenum usage)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glBufferData (target, size, data, usage);
}
void opengl::context::bufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid const* data)
{
#ifndef NOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS
  verify_context_and_check_for_gl_errors const _ (_current_context, BOOST_CURRENT_FUNCTION);
#endif
  return _current_context->functions()->glBufferSubData (target, offset, size, data);
}

template<GLenum target>
void opengl::context::bufferSubData (GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid const* data)
{
  GLuint old;
  gl.getIntegerv ( target == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING
  : target == GL_DRAW_INDIRECT_BUFFER ? GL_DRAW_INDIRECT_BUFFER_BINDING
  : target == GL_ELEMENT_ARRAY_BUFFER ? GL_ELEMENT_ARRAY_BUFFER_BINDING
  : target == GL_PIXEL_PACK_BUFFER ? GL_PIXEL_PACK_BUFFER_BINDING
  : target == GL_PIXEL_UNPACK_BUFFER ? GL_PIXEL_UNPACK_BUFFER_BINDING
  : target == GL_TRANSFORM_FEEDBACK_BUFFER ? GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
  : target == GL_UNIFORM_BUFFER ? GL_UNIFORM_BUFFER_BINDING
  : throw std::logic_error ("bad bind target")
    , reinterpret_cast<GLint*> (&old)
  );

  gl.bindBuffer (target, buffer);
  bufferSubData (target, offset, size, data);
  gl.bindBuffer (target, old);
}

template<GLenum target, typename T>
void opengl::context::bufferSubData(GLuint buffer, GLintptr offset, std::vector<T> const& data)
{
  GLuint old;
  gl.getIntegerv ( target == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING
                   : target == GL_DRAW_INDIRECT_BUFFER ? GL_DRAW_INDIRECT_BUFFER_BINDING
                   : target == GL_ELEMENT_ARRAY_BUFFER ? GL_ELEMENT_ARRAY_BUFFER_BINDING
                   : target == GL_PIXEL_PACK_BUFFER ? GL_PIXEL_PACK_BUFFER_BINDING
                   : target == GL_PIXEL_UNPACK_BUFFER ? GL_PIXEL_UNPACK_BUFFER_BINDING
                   : target == GL_TRANSFORM_FEEDBACK_BUFFER ? GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
                   : target == GL_UNIFORM_BUFFER ? GL_UNIFORM_BUFFER_BINDING
                   : throw std::logic_error ("bad bind target")
    , reinterpret_cast<GLint*> (&old)
  );
  gl.bindBuffer (target, buffer);
  bufferSubData(target, offset, sizeof(T) * data.size(), data.data());
  gl.bindBuffer (target, old);
}

template void opengl::context::bufferData<GL_ARRAY_BUFFER, float>(GLuint buffer, std::vector<float> const& data, GLenum usage);
template void opengl::context::bufferData<GL_ARRAY_BUFFER, math::vector_2d>(GLuint buffer, std::vector<math::vector_2d> const& data, GLenum usage);
template void opengl::context::bufferData<GL_ARRAY_BUFFER, math::vector_3d>(GLuint buffer, std::vector<math::vector_3d> const& data, GLenum usage);
template void opengl::context::bufferData<GL_ARRAY_BUFFER, math::vector_4d>(GLuint buffer, std::vector<math::vector_4d> const& data, GLenum usage);
template void opengl::context::bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint8_t>(GLuint buffer, std::vector<std::uint8_t> const& data, GLenum usage);
template void opengl::context::bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(GLuint buffer, std::vector<std::uint16_t> const& data, GLenum usage);
template void opengl::context::bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint32_t>(GLuint buffer, std::vector<std::uint32_t> const& data, GLenum usage);

template void opengl::context::bufferSubData<GL_ARRAY_BUFFER, float>(GLuint buffer, GLintptr offset, std::vector<float> const& data);
template void opengl::context::bufferSubData<GL_ARRAY_BUFFER, math::vector_2d>(GLuint buffer, GLintptr offset, std::vector<math::vector_2d> const& data);
template void opengl::context::bufferSubData<GL_ARRAY_BUFFER, math::vector_3d>(GLuint buffer, GLintptr offset, std::vector<math::vector_3d> const& data);
template void opengl::context::bufferSubData<GL_ARRAY_BUFFER, math::vector_4d>(GLuint buffer, GLintptr offset, std::vector<math::vector_4d> const& data);
template void opengl::context::bufferSubData<GL_ARRAY_BUFFER>(GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid const* data);


void opengl::context::drawElements (GLenum mode, GLuint index_buffer, GLsizei count, GLenum type, GLvoid const* indices)
{
  GLuint old;
  gl.getIntegerv (GL_ELEMENT_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&old));
  gl.bindBuffer (GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  drawElements (mode, count, type, indices);
  gl.bindBuffer (GL_ELEMENT_ARRAY_BUFFER, old);
}

#endif //NOGGIT_CONTEXT_INL
