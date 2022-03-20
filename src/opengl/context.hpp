// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <opengl/types.hpp>
#include <QtGui/QOpenGLFunctions_4_1_Core>

// NOGGIT_FORCEINLINE ---------------------------------------------//
// Macro to use in place of 'inline' to force a function to be inline
#  if defined(_MSC_VER)
#    define NOGGIT_FORCEINLINE __forceinline
#  elif defined(__GNUC__) && __GNUC__ > 3
     // Clang also defines __GNUC__ (as 4)
#    define NOGGIT_FORCEINLINE inline __attribute__ ((__always_inline__))
#  else
#    define NOGGIT_FORCEINLINE inline
#  endif


namespace OpenGL
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

    NOGGIT_FORCEINLINE void enable (GLenum);
    NOGGIT_FORCEINLINE void disable (GLenum);
    NOGGIT_FORCEINLINE GLboolean isEnabled (GLenum);

    NOGGIT_FORCEINLINE void viewport (GLint x, GLint y, GLsizei width, GLsizei height);

    NOGGIT_FORCEINLINE void depthFunc (GLenum);
    NOGGIT_FORCEINLINE void depthMask (GLboolean);
    NOGGIT_FORCEINLINE void blendFunc (GLenum, GLenum);

    NOGGIT_FORCEINLINE void clear (GLenum);
    NOGGIT_FORCEINLINE void clearColor (GLfloat, GLfloat, GLfloat, GLfloat);

    NOGGIT_FORCEINLINE void readBuffer (GLenum);
    NOGGIT_FORCEINLINE void readPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data);

    NOGGIT_FORCEINLINE void lineWidth (GLfloat);

    NOGGIT_FORCEINLINE void pointParameterf (GLenum pname, GLfloat param);
    NOGGIT_FORCEINLINE void pointParameteri (GLenum pname, GLint param);
    NOGGIT_FORCEINLINE void pointParameterfv (GLenum pname, GLfloat const* param);
    NOGGIT_FORCEINLINE void pointParameteriv (GLenum pname, GLint const* param);
    NOGGIT_FORCEINLINE void pointSize (GLfloat);

    NOGGIT_FORCEINLINE void hint (GLenum, GLenum);
    NOGGIT_FORCEINLINE void polygonMode (GLenum face, GLenum mode);

    NOGGIT_FORCEINLINE void genTextures (GLuint, GLuint*);
    NOGGIT_FORCEINLINE void deleteTextures (GLuint, GLuint*);
    NOGGIT_FORCEINLINE void bindTexture (GLenum target, GLuint);
    NOGGIT_FORCEINLINE void texImage2D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid const* data);
    NOGGIT_FORCEINLINE void texSubImage2D(GLenum target,
                                         GLint level,
                                         GLint xoffset,
                                         GLint yoffset,
                                         GLsizei width,
                                         GLsizei height,
                                         GLenum format,
                                         GLenum type,
                                         const void * pixels);
    NOGGIT_FORCEINLINE void compressedTexSubImage2D( GLenum target,
                                                    GLint level,
                                                    GLint xoffset,
                                                    GLint yoffset,
                                                    GLsizei width,
                                                    GLsizei height,
                                                    GLenum format,
                                                    GLsizei imageSize,
                                                    const void * data);
    NOGGIT_FORCEINLINE void texImage3D (GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLvoid const* data);
    NOGGIT_FORCEINLINE void texSubImage3D (GLenum target,
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
    NOGGIT_FORCEINLINE void compressedTexSubImage3D (GLenum target,
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
    NOGGIT_FORCEINLINE void compressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, GLvoid const* data);
    NOGGIT_FORCEINLINE void compressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, 	GLsizei depth, GLint border, GLsizei imageSize, GLvoid const* data);
    NOGGIT_FORCEINLINE void generateMipmap (GLenum);
    NOGGIT_FORCEINLINE void activeTexture (GLenum);

    NOGGIT_FORCEINLINE void texParameteri (GLenum target, GLenum pname, GLint param);
    NOGGIT_FORCEINLINE void texParameterf (GLenum target, GLenum pname, GLfloat param);
    NOGGIT_FORCEINLINE void texParameteriv (GLenum target, GLenum pname, GLint const* params);
    NOGGIT_FORCEINLINE void texParameterfv (GLenum target, GLenum pname, GLfloat const* params);
    NOGGIT_FORCEINLINE void texBuffer(	GLenum target, GLenum internalformat, GLuint buffer);

    NOGGIT_FORCEINLINE void genVertexArrays (GLuint, GLuint*);
    NOGGIT_FORCEINLINE void deleteVertexArray (GLuint, GLuint*);
    NOGGIT_FORCEINLINE void bindVertexArray (GLenum);

    NOGGIT_FORCEINLINE void genBuffers (GLuint, GLuint*);
    NOGGIT_FORCEINLINE void deleteBuffers (GLuint, GLuint*);
    NOGGIT_FORCEINLINE void bindBuffer (GLenum, GLuint);
    NOGGIT_FORCEINLINE void bindBufferRange (GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
    NOGGIT_FORCEINLINE void bufferData (GLenum target, GLsizeiptr size, GLvoid const* data, GLenum usage);
    NOGGIT_FORCEINLINE GLvoid* mapBuffer (GLenum target, GLenum access);
    NOGGIT_FORCEINLINE GLboolean unmapBuffer (GLenum);
    NOGGIT_FORCEINLINE void drawElements (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices);
    NOGGIT_FORCEINLINE void drawElementsInstanced (GLenum mode, GLsizei count, GLenum type, GLvoid const* indices, GLsizei instancecount);
    NOGGIT_FORCEINLINE void drawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, GLvoid const* indices);

    NOGGIT_FORCEINLINE void genPrograms (GLsizei programs, GLuint*);
    NOGGIT_FORCEINLINE void deletePrograms (GLsizei programs, GLuint*);
    NOGGIT_FORCEINLINE void bindProgram (GLenum, GLuint);
    NOGGIT_FORCEINLINE void programString (GLenum target, GLenum format, GLsizei len, GLvoid const* pointer);
    NOGGIT_FORCEINLINE void getProgramiv (GLuint program, GLenum pname, GLint* params);
    NOGGIT_FORCEINLINE void programLocalParameter4f (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);

    NOGGIT_FORCEINLINE void getBooleanv (GLenum, GLboolean*);
    NOGGIT_FORCEINLINE void getDoublev (GLenum, GLdouble*);
    NOGGIT_FORCEINLINE void getFloatv (GLenum, GLfloat*);
    NOGGIT_FORCEINLINE void getIntegerv (GLenum, GLint*);

    NOGGIT_FORCEINLINE GLubyte const* getString (GLenum);

    NOGGIT_FORCEINLINE GLuint createShader (GLenum shader_type);
    NOGGIT_FORCEINLINE void deleteShader (GLuint shader);
    NOGGIT_FORCEINLINE void shaderSource (GLuint shader, GLsizei count, GLchar const** string, GLint const* length);
    NOGGIT_FORCEINLINE void compile_shader (GLuint shader);
    NOGGIT_FORCEINLINE GLint get_shader (GLuint shader, GLenum pname);

    NOGGIT_FORCEINLINE GLuint createProgram();
    NOGGIT_FORCEINLINE void deleteProgram (GLuint program);
    NOGGIT_FORCEINLINE void attachShader (GLuint program, GLuint shader);
    NOGGIT_FORCEINLINE void detachShader (GLuint program, GLuint shader);
    NOGGIT_FORCEINLINE void link_program (GLuint program);
    NOGGIT_FORCEINLINE void useProgram (GLuint program);
    NOGGIT_FORCEINLINE void validate_program (GLuint program);
    NOGGIT_FORCEINLINE GLint get_program (GLuint program, GLenum pname);
    NOGGIT_FORCEINLINE std::string get_program_info_log(GLuint program);

    NOGGIT_FORCEINLINE GLint getAttribLocation (GLuint program, GLchar const* name);
    NOGGIT_FORCEINLINE void vertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid const* pointer);
    NOGGIT_FORCEINLINE void vertexAttribIPointer (GLuint index,
                                                 GLint size,
                                                 GLenum type,
                                                 GLsizei stride,
                                                 const void * pointer);
    NOGGIT_FORCEINLINE void vertexAttribDivisor (GLuint index, GLuint divisor);
    NOGGIT_FORCEINLINE void enableVertexAttribArray (GLuint index);
    NOGGIT_FORCEINLINE void disableVertexAttribArray (GLuint index);

    NOGGIT_FORCEINLINE GLint getUniformLocation (GLuint program, GLchar const* name);
    NOGGIT_FORCEINLINE GLint getUniformBlockIndex (GLuint program, GLchar const* name);

    NOGGIT_FORCEINLINE void uniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

    NOGGIT_FORCEINLINE void uniform1i (GLint location, GLint value);
    NOGGIT_FORCEINLINE void uniform1f (GLint location, GLfloat value);
    NOGGIT_FORCEINLINE void uniform1iv (GLint location, GLsizei count, GLint const* value);
    NOGGIT_FORCEINLINE void uniform2iv (GLint location, GLsizei count, GLint const* value);
    NOGGIT_FORCEINLINE void uniform2fv (GLint location, GLsizei count, GLfloat const* value);
    NOGGIT_FORCEINLINE void uniform3fv (GLint location, GLsizei count, GLfloat const* value);
    NOGGIT_FORCEINLINE void uniform4fv (GLint location, GLsizei count, GLfloat const* value);
    NOGGIT_FORCEINLINE void uniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, GLfloat const* value);

    NOGGIT_FORCEINLINE void clearStencil (GLint);
    NOGGIT_FORCEINLINE void stencilFunc (GLenum func, GLint ref, GLuint mask);
    NOGGIT_FORCEINLINE void stencilOp (GLenum sfail, GLenum dpfail, GLenum dppass);
    NOGGIT_FORCEINLINE void colorMask (GLboolean r, GLboolean g, GLboolean b, GLboolean a);

    NOGGIT_FORCEINLINE void polygonOffset (GLfloat factor, GLfloat units);

    NOGGIT_FORCEINLINE void genFramebuffers (GLsizei n, GLuint *ids);
    NOGGIT_FORCEINLINE void bindFramebuffer (GLenum target, GLuint framebuffer);
    NOGGIT_FORCEINLINE void framebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

    NOGGIT_FORCEINLINE void genRenderbuffers (GLsizei n, GLuint* renderbuffers);
    NOGGIT_FORCEINLINE void bindRenderbuffer (GLenum target, GLuint renderbuffer);
    NOGGIT_FORCEINLINE void renderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    NOGGIT_FORCEINLINE void framebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

    NOGGIT_FORCEINLINE void genQueries(GLsizei n, GLuint* ids);
    NOGGIT_FORCEINLINE void deleteQueries(GLsizei n, GLuint* ids);
    NOGGIT_FORCEINLINE void beginQuery(GLenum target, GLuint id);
    NOGGIT_FORCEINLINE void endQuery(GLenum target);
    NOGGIT_FORCEINLINE void getQueryObjectiv(GLuint id, GLenum pname, GLint* params);



    template<GLenum target>
    NOGGIT_FORCEINLINE void bufferData (GLuint buffer, GLsizeiptr size, GLvoid const* data, GLenum usage);
    template<GLenum target, typename T>
    NOGGIT_FORCEINLINE void bufferData(GLuint buffer, std::vector<T> const& data, GLenum usage);

    NOGGIT_FORCEINLINE void drawElements (GLenum mode, GLuint index_buffer, GLsizei count, GLenum type, GLvoid const* indices);

    NOGGIT_FORCEINLINE void drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);

    QOpenGLContext* getCurrentContext();

    NOGGIT_FORCEINLINE void bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);

    template<GLenum target>
    NOGGIT_FORCEINLINE void bufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);

    template<GLenum target, typename T>
    NOGGIT_FORCEINLINE void bufferSubData(GLuint buffer, GLintptr offset, const std::vector<T> &data);
  };
}

extern OpenGL::context gl;
