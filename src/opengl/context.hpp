// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/types.hpp>
#include <boost/config/detail/suffix.hpp>

#include <QtGui/QOpenGLFunctions_4_1_Core>

namespace opengl
{
  struct context
  {
    struct scoped_setter
    {
      scoped_setter (context&, QOpenGLContext*);
      ~scoped_setter();

      scoped_setter (scoped_setter const&) = delete;
      scoped_setter (scoped_setter&&) = delete;
      scoped_setter& operator= (scoped_setter const&) = delete;
      scoped_setter& operator= (scoped_setter&&) = delete;

    private:
      context& _context;
      QOpenGLContext* _old_context;
      QOpenGLFunctions_4_1_Core* _old_core_func;
    };

    struct save_current_context
    {
      save_current_context (context&);
      ~save_current_context();

      save_current_context (save_current_context const&) = delete;
      save_current_context (save_current_context&&) = delete;
      save_current_context& operator= (save_current_context const&) = delete;
      save_current_context& operator= (save_current_context&&) = delete;

    private:
      bool _is_current;
      QOpenGLContext* _gl_context;
      QSurface* _surface;
    };

    QOpenGLContext* _current_context = nullptr;
    QOpenGLFunctions_4_1_Core* _4_1_core_func = nullptr;

    BOOST_FORCEINLINE void enable (GLenum);
    BOOST_FORCEINLINE void disable (GLenum);
    BOOST_FORCEINLINE GLboolean isEnabled (GLenum);

    BOOST_FORCEINLINE void viewport (GLint x, GLint y, GLsizei width, GLsizei height);

    BOOST_FORCEINLINE void depthFunc (GLenum);
    BOOST_FORCEINLINE void depthMask (GLboolean);
    BOOST_FORCEINLINE void blendFunc (GLenum, GLenum);

    BOOST_FORCEINLINE void clear (GLenum);
    BOOST_FORCEINLINE void clearColor (GLfloat, GLfloat, GLfloat, GLfloat);

    BOOST_FORCEINLINE void readBuffer (GLenum);
    BOOST_FORCEINLINE void readPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data);

    BOOST_FORCEINLINE void lineWidth (GLfloat);

    BOOST_FORCEINLINE void pointParameterf (GLenum pname, GLfloat param);
    BOOST_FORCEINLINE void pointParameteri (GLenum pname, GLint param);
    BOOST_FORCEINLINE void pointParameterfv (GLenum pname, GLfloat const* param);
    BOOST_FORCEINLINE void pointParameteriv (GLenum pname, GLint const* param);
    BOOST_FORCEINLINE void pointSize (GLfloat);

    BOOST_FORCEINLINE void hint (GLenum, GLenum);
    BOOST_FORCEINLINE void polygonMode (GLenum face, GLenum mode);

    BOOST_FORCEINLINE void genTextures (GLuint, GLuint*);
    BOOST_FORCEINLINE void deleteTextures (GLuint, GLuint*);
    BOOST_FORCEINLINE void bindTexture (GLenum target, GLuint);
    BOOST_FORCEINLINE void texImage2D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid const* data);
    BOOST_FORCEINLINE void texSubImage2D(GLenum target,
                                         GLint level,
                                         GLint xoffset,
                                         GLint yoffset,
                                         GLsizei width,
                                         GLsizei height,
                                         GLenum format,
                                         GLenum type,
                                         const void * pixels);
    BOOST_FORCEINLINE void texImage3D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLvoid const* data);
    BOOST_FORCEINLINE void texSubImage3D (GLenum target,
                                          GLint level,
                                          GLint xoffset,
                                          GLint yoffset,
                                          GLint zoffset,
                                          GLsizei width,
                                          GLsizei height,
                                          GLsizei depth,
                                          GLenum format,
                                          GLenum type,
                                          const void * pixels);
    BOOST_FORCEINLINE void compressedTexSubImage3D (GLenum target,
                                                      GLint level,
                                                      GLint xoffset,
                                                      GLint yoffset,
                                                      GLint zoffset,
                                                      GLsizei width,
                                                      GLsizei height,
                                                      GLsizei depth,
                                                      GLenum format,
                                                      GLsizei imageSize,
                                                      const void * data);
    BOOST_FORCEINLINE void compressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid const* data);
    BOOST_FORCEINLINE void compressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, 	GLsizei depth, GLint border, GLsizei imageSize, GLvoid const* data);
    BOOST_FORCEINLINE void generateMipmap (GLenum);
    BOOST_FORCEINLINE void activeTexture (GLenum);

    BOOST_FORCEINLINE void texParameteri (GLenum target, GLenum pname, GLint param);
    BOOST_FORCEINLINE void texParameterf (GLenum target, GLenum pname, GLfloat param);
    BOOST_FORCEINLINE void texParameteriv (GLenum target, GLenum pname, GLint const* params);
    BOOST_FORCEINLINE void texParameterfv (GLenum target, GLenum pname, GLfloat const* params);

    BOOST_FORCEINLINE void genVertexArrays (GLuint, GLuint*);
    BOOST_FORCEINLINE void deleteVertexArray (GLuint, GLuint*);
    BOOST_FORCEINLINE void bindVertexArray (GLenum);

    BOOST_FORCEINLINE void genBuffers (GLuint, GLuint*);
    BOOST_FORCEINLINE void deleteBuffers (GLuint, GLuint*);
    BOOST_FORCEINLINE void bindBuffer (GLenum, GLuint);
    BOOST_FORCEINLINE void bindBufferRange (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
    BOOST_FORCEINLINE void bufferData (GLenum target, GLsizeiptr size, GLvoid const* data, GLenum usage);
    BOOST_FORCEINLINE GLvoid* mapBuffer (GLenum target, GLenum access);
    BOOST_FORCEINLINE GLboolean unmapBuffer (GLenum);
    BOOST_FORCEINLINE void drawElements (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices);
    BOOST_FORCEINLINE void drawElementsInstanced (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices, GLsizei instancecount);
    BOOST_FORCEINLINE void drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid const* indices);

    BOOST_FORCEINLINE void genPrograms (GLsizei programs, GLuint*);
    BOOST_FORCEINLINE void deletePrograms (GLsizei programs, GLuint*);
    BOOST_FORCEINLINE void bindProgram (GLenum, GLuint);
    BOOST_FORCEINLINE void programString (GLenum target, GLenum format, GLsizei len, GLvoid const* pointer);
    BOOST_FORCEINLINE void getProgramiv (GLuint program, GLenum pname, GLint* params);
    BOOST_FORCEINLINE void programLocalParameter4f (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);

    BOOST_FORCEINLINE void getBooleanv (GLenum, GLboolean*);
    BOOST_FORCEINLINE void getDoublev (GLenum, GLdouble*);
    BOOST_FORCEINLINE void getFloatv (GLenum, GLfloat*);
    BOOST_FORCEINLINE void getIntegerv (GLenum, GLint*);

    BOOST_FORCEINLINE GLubyte const* getString (GLenum);

    BOOST_FORCEINLINE GLuint createShader (GLenum shader_type);
    BOOST_FORCEINLINE void deleteShader (GLuint shader);
    BOOST_FORCEINLINE void shaderSource (GLuint shader, GLsizei count, GLchar const** string, GLint const* length);
    BOOST_FORCEINLINE void compile_shader (GLuint shader);
    BOOST_FORCEINLINE GLint get_shader (GLuint shader, GLenum pname);

    BOOST_FORCEINLINE GLuint createProgram();
    BOOST_FORCEINLINE void deleteProgram (GLuint program);
    BOOST_FORCEINLINE void attachShader (GLuint program, GLuint shader);
    BOOST_FORCEINLINE void detachShader (GLuint program, GLuint shader);
    BOOST_FORCEINLINE void link_program (GLuint program);
    BOOST_FORCEINLINE void useProgram (GLuint program);
    BOOST_FORCEINLINE void validate_program (GLuint program);
    BOOST_FORCEINLINE GLint get_program (GLuint program, GLenum pname);
    BOOST_FORCEINLINE std::string get_program_info_log(GLuint program);

    BOOST_FORCEINLINE GLint getAttribLocation (GLuint program, GLchar const* name);
    BOOST_FORCEINLINE void vertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid const* pointer);
    BOOST_FORCEINLINE void vertexAttribDivisor (GLuint index, GLuint divisor);
    BOOST_FORCEINLINE void enableVertexAttribArray (GLuint index);
    BOOST_FORCEINLINE void disableVertexAttribArray (GLuint index);

    BOOST_FORCEINLINE GLint getUniformLocation (GLuint program, GLchar const* name);
    BOOST_FORCEINLINE GLint getUniformBlockIndex (GLuint program, GLchar const* name);

    BOOST_FORCEINLINE void uniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

    BOOST_FORCEINLINE void uniform1i (GLint location, GLint value);
    BOOST_FORCEINLINE void uniform1f (GLint location, GLfloat value);
    BOOST_FORCEINLINE void uniform1iv (GLint location, GLsizei count, GLint const* value);
    BOOST_FORCEINLINE void uniform2iv (GLint location, GLsizei count, GLint const* value);
    BOOST_FORCEINLINE void uniform2fv (GLint location, GLsizei count, GLfloat const* value);
    BOOST_FORCEINLINE void uniform3fv (GLint location, GLsizei count, GLfloat const* value);
    BOOST_FORCEINLINE void uniform4fv (GLint location, GLsizei count, GLfloat const* value);
    BOOST_FORCEINLINE void uniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, GLfloat const* value);

    BOOST_FORCEINLINE void clearStencil (GLint);
    BOOST_FORCEINLINE void stencilFunc (GLenum func, GLint ref, GLuint mask);
    BOOST_FORCEINLINE void stencilOp (GLenum sfail, GLenum dpfail, GLenum dppass);
    BOOST_FORCEINLINE void colorMask (GLboolean r, GLboolean g, GLboolean b, GLboolean a);

    BOOST_FORCEINLINE void polygonOffset (GLfloat factor, GLfloat units);

    BOOST_FORCEINLINE void genFramebuffers (GLsizei n, GLuint *ids);
    BOOST_FORCEINLINE void bindFramebuffer (GLenum target, GLuint framebuffer);
    BOOST_FORCEINLINE void framebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

    BOOST_FORCEINLINE void genRenderbuffers (GLsizei n, GLuint *renderbuffers);
    BOOST_FORCEINLINE void bindRenderbuffer (GLenum target, GLuint renderbuffer);
    BOOST_FORCEINLINE void renderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    BOOST_FORCEINLINE void framebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

    template<GLenum target>
    BOOST_FORCEINLINE void bufferData (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);
    template<GLenum target, typename T>
    BOOST_FORCEINLINE void bufferData(GLuint buffer, std::vector<T> const& data, GLenum usage);

    BOOST_FORCEINLINE void drawElements (GLenum mode, GLuint index_buffer, GLsizei count, GLenum type, GLvoid const* indices);

    BOOST_FORCEINLINE void drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);

    QOpenGLContext* getCurrentContext();

    BOOST_FORCEINLINE void bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);

    template<GLenum target>
    BOOST_FORCEINLINE void bufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);

    template<GLenum target, typename T>
    BOOST_FORCEINLINE void bufferSubData(GLuint buffer, GLintptr offset, const std::vector<T> &data);
  };
}

extern opengl::context gl;
