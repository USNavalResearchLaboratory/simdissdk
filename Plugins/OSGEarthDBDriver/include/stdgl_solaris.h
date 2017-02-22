/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef STDGL_h
#define STDGL_h

#include <stdio.h>
#include <GL/glu.h>

#ifdef __USE_STD_GL_ERROR__

#ifndef NDEBUG

inline void glAccumError(GLenum  op, GLfloat  value, const char *filename, int lineno)
{
  glAccum(op, value);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glAccum]: %s\n", filename, lineno, gluErrorString(error));
}
#define glAccum(op, value) glAccumError(op, value, __FILE__, __LINE__)

inline void glActiveTextureError(GLenum  texture, const char *filename, int lineno)
{
  glActiveTexture(texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glActiveTexture]: %s\n", filename, lineno, gluErrorString(error));
}
#define glActiveTexture(texture) glActiveTextureError(texture, __FILE__, __LINE__)

inline void glAlphaFuncError(GLenum  func, GLclampf  ref, const char *filename, int lineno)
{
  glAlphaFunc(func, ref);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glAlphaFunc]: %s\n", filename, lineno, gluErrorString(error));
}
#define glAlphaFunc(func, ref) glAlphaFuncError(func, ref, __FILE__, __LINE__)

inline void glArrayElementError(GLint  i, const char *filename, int lineno)
{
  glArrayElement(i);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glArrayElement]: %s\n", filename, lineno, gluErrorString(error));
}
#define glArrayElement(i) glArrayElementError(i, __FILE__, __LINE__)

inline void glBeginError(GLenum  mode, const char *filename, int lineno)
{
  glBegin(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBegin]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBegin(mode) glBeginError(mode, __FILE__, __LINE__)

inline void glBitmapError(GLsizei  width, GLsizei  height, GLfloat  xorig, GLfloat  yorig, GLfloat  xmove, GLfloat  ymove, const  GLubyte * bitmap, const char *filename, int lineno)
{
  glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBitmap]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap) glBitmapError(width, height, xorig, yorig, xmove, ymove, bitmap, __FILE__, __LINE__)

inline void glBlendColorError(GLclampf  red, GLclampf  green, GLclampf  blue, GLclampf  alpha, const char *filename, int lineno)
{
  glBlendColor(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendColor]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendColor(red, green, blue, alpha) glBlendColorError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glBlendEquationError(GLenum  mode, const char *filename, int lineno)
{
  glBlendEquation(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendEquation]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendEquation(mode) glBlendEquationError(mode, __FILE__, __LINE__)

inline void glBlendFuncError(GLenum  sfactor, GLenum  dfactor, const char *filename, int lineno)
{
  glBlendFunc(sfactor, dfactor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendFunc]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendFunc(sfactor, dfactor) glBlendFuncError(sfactor, dfactor, __FILE__, __LINE__)

inline void glCallListError(GLuint  list, const char *filename, int lineno)
{
  glCallList(list);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCallList]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCallList(list) glCallListError(list, __FILE__, __LINE__)

inline void glCallListsError(GLsizei  n, GLenum  type, const  GLvoid * lists, const char *filename, int lineno)
{
  glCallLists(n, type, lists);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCallLists]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCallLists(n, type, lists) glCallListsError(n, type, lists, __FILE__, __LINE__)

inline void glClearError(GLbitfield  mask, const char *filename, int lineno)
{
  glClear(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClear]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClear(mask) glClearError(mask, __FILE__, __LINE__)

inline void glClearAccumError(GLfloat  red, GLfloat  green, GLfloat  blue, GLfloat  alpha, const char *filename, int lineno)
{
  glClearAccum(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearAccum]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearAccum(red, green, blue, alpha) glClearAccumError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glClearColorError(GLclampf  red, GLclampf  green, GLclampf  blue, GLclampf  alpha, const char *filename, int lineno)
{
  glClearColor(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearColor]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearColor(red, green, blue, alpha) glClearColorError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glClearDepthError(GLclampd  depth, const char *filename, int lineno)
{
  glClearDepth(depth);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearDepth]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearDepth(depth) glClearDepthError(depth, __FILE__, __LINE__)

inline void glClearIndexError(GLfloat  c, const char *filename, int lineno)
{
  glClearIndex(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearIndex]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearIndex(c) glClearIndexError(c, __FILE__, __LINE__)

inline void glClearStencilError(GLint  s, const char *filename, int lineno)
{
  glClearStencil(s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearStencil]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearStencil(s) glClearStencilError(s, __FILE__, __LINE__)

inline void glClientActiveTextureError(GLenum  texture, const char *filename, int lineno)
{
  glClientActiveTexture(texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClientActiveTexture]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClientActiveTexture(texture) glClientActiveTextureError(texture, __FILE__, __LINE__)

inline void glClipPlaneError(GLenum  plane, const  GLdouble * equation, const char *filename, int lineno)
{
  glClipPlane(plane, equation);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClipPlane]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClipPlane(plane, equation) glClipPlaneError(plane, equation, __FILE__, __LINE__)

inline void glColorMaskError(GLboolean  red, GLboolean  green, GLboolean  blue, GLboolean  alpha, const char *filename, int lineno)
{
  glColorMask(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorMask]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorMask(red, green, blue, alpha) glColorMaskError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColorMaterialError(GLenum  face, GLenum  mode, const char *filename, int lineno)
{
  glColorMaterial(face, mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorMaterial]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorMaterial(face, mode) glColorMaterialError(face, mode, __FILE__, __LINE__)

inline void glColorPointerError(GLint  size, GLenum  type, GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glColorPointer(size, type, stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorPointer(size, type, stride, pointer) glColorPointerError(size, type, stride, pointer, __FILE__, __LINE__)

inline void glColorSubTableError(GLenum  target, GLsizei  start, GLsizei  count, GLenum  format, GLenum  type, const  GLvoid * data, const char *filename, int lineno)
{
  glColorSubTable(target, start, count, format, type, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorSubTable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorSubTable(target, start, count, format, type, data) glColorSubTableError(target, start, count, format, type, data, __FILE__, __LINE__)

inline void glColorTableError(GLenum  target, GLenum  internalformat, GLsizei  width, GLenum  format, GLenum  type, const  GLvoid * table, const char *filename, int lineno)
{
  glColorTable(target, internalformat, width, format, type, table);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTable(target, internalformat, width, format, type, table) glColorTableError(target, internalformat, width, format, type, table, __FILE__, __LINE__)

inline void glColorTableParameterfvError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glColorTableParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTableParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTableParameterfv(target, pname, params) glColorTableParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glColorTableParameterivError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glColorTableParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTableParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTableParameteriv(target, pname, params) glColorTableParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glCompressedTexImage1DError(GLenum  target, int  GLlevel, GLenum  internalformat, GLsizei  width, GLint  border, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexImage1D(target, GLlevel, internalformat, width, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage1D(target, GLlevel, internalformat, width, border, imageSize, data) glCompressedTexImage1DError(target, GLlevel, internalformat, width, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexImage2DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLint  border, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data) glCompressedTexImage2DError(target, level, internalformat, width, height, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexImage3DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLsizei  depth, GLint  border, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data) glCompressedTexImage3DError(target, level, internalformat, width, height, depth, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage1DError(GLenum  target, GLint  level, GLint  xoffset, GLsizei  width, GLenum  format, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data) glCompressedTexSubImage1DError(target, level, xoffset, width, format, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage2DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLsizei  width, GLsizei  height, GLenum  format, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data) glCompressedTexSubImage2DError(target, level, xoffset, yoffset, width, height, format, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage3DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLsizei  width, GLsizei  height, GLsizei  depth, GLenum  format, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) glCompressedTexSubImage3DError(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data, __FILE__, __LINE__)

inline void glConvolutionFilter1DError(GLenum  target, GLenum  internalformat, GLsizei  width, GLenum  format, GLenum  type, const  GLvoid * image, const char *filename, int lineno)
{
  glConvolutionFilter1D(target, internalformat, width, format, type, image);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionFilter1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionFilter1D(target, internalformat, width, format, type, image) glConvolutionFilter1DError(target, internalformat, width, format, type, image, __FILE__, __LINE__)

inline void glConvolutionFilter2DError(GLenum  target, GLenum  internalformat, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, const  GLvoid * image, const char *filename, int lineno)
{
  glConvolutionFilter2D(target, internalformat, width, height, format, type, image);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionFilter2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionFilter2D(target, internalformat, width, height, format, type, image) glConvolutionFilter2DError(target, internalformat, width, height, format, type, image, __FILE__, __LINE__)

inline void glConvolutionParameterfError(GLenum  target, GLenum  pname, GLfloat  params, const char *filename, int lineno)
{
  glConvolutionParameterf(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameterf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameterf(target, pname, params) glConvolutionParameterfError(target, pname, params, __FILE__, __LINE__)

inline void glConvolutionParameterfvError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glConvolutionParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameterfv(target, pname, params) glConvolutionParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glConvolutionParameteriError(GLenum  target, GLenum  pname, GLint  params, const char *filename, int lineno)
{
  glConvolutionParameteri(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameteri]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameteri(target, pname, params) glConvolutionParameteriError(target, pname, params, __FILE__, __LINE__)

inline void glConvolutionParameterivError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glConvolutionParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameteriv(target, pname, params) glConvolutionParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glCopyColorSubTableError(GLenum  target, GLsizei  start, GLint  x, GLint  y, GLsizei  width, const char *filename, int lineno)
{
  glCopyColorSubTable(target, start, x, y, width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyColorSubTable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyColorSubTable(target, start, x, y, width) glCopyColorSubTableError(target, start, x, y, width, __FILE__, __LINE__)

inline void glCopyColorTableError(GLenum  target, GLenum  internalformat, GLint  x, GLint  y, GLsizei  width, const char *filename, int lineno)
{
  glCopyColorTable(target, internalformat, x, y, width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyColorTable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyColorTable(target, internalformat, x, y, width) glCopyColorTableError(target, internalformat, x, y, width, __FILE__, __LINE__)

inline void glCopyConvolutionFilter1DError(GLenum  target, GLenum  internalformat, GLint  x, GLint  y, GLsizei  width, const char *filename, int lineno)
{
  glCopyConvolutionFilter1D(target, internalformat, x, y, width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyConvolutionFilter1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyConvolutionFilter1D(target, internalformat, x, y, width) glCopyConvolutionFilter1DError(target, internalformat, x, y, width, __FILE__, __LINE__)

inline void glCopyConvolutionFilter2DError(GLenum  target, GLenum  internalformat, GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glCopyConvolutionFilter2D(target, internalformat, x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyConvolutionFilter2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyConvolutionFilter2D(target, internalformat, x, y, width, height) glCopyConvolutionFilter2DError(target, internalformat, x, y, width, height, __FILE__, __LINE__)

inline void glCopyPixelsError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLenum  type, const char *filename, int lineno)
{
  glCopyPixels(x, y, width, height, type);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyPixels]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyPixels(x, y, width, height, type) glCopyPixelsError(x, y, width, height, type, __FILE__, __LINE__)

inline void glCopyTexImage1DError(GLenum  target, GLint  level, GLenum  internalFormat, GLint  x, GLint  y, GLsizei  width, GLint  border, const char *filename, int lineno)
{
  glCopyTexImage1D(target, level, internalFormat, x, y, width, border);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexImage1D(target, level, internalFormat, x, y, width, border) glCopyTexImage1DError(target, level, internalFormat, x, y, width, border, __FILE__, __LINE__)

inline void glCopyTexImage2DError(GLenum  target, GLint  level, GLenum  internalFormat, GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLint  border, const char *filename, int lineno)
{
  glCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexImage2D(target, level, internalFormat, x, y, width, height, border) glCopyTexImage2DError(target, level, internalFormat, x, y, width, height, border, __FILE__, __LINE__)

inline void glCopyTexSubImage1DError(GLenum  target, GLint  level, GLint  xoffset, GLint  x, GLint  y, GLsizei  width, const char *filename, int lineno)
{
  glCopyTexSubImage1D(target, level, xoffset, x, y, width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexSubImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexSubImage1D(target, level, xoffset, x, y, width) glCopyTexSubImage1DError(target, level, xoffset, x, y, width, __FILE__, __LINE__)

inline void glCopyTexSubImage2DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexSubImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height) glCopyTexSubImage2DError(target, level, xoffset, yoffset, x, y, width, height, __FILE__, __LINE__)

inline void glCopyTexSubImage3DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexSubImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height) glCopyTexSubImage3DError(target, level, xoffset, yoffset, zoffset, x, y, width, height, __FILE__, __LINE__)

inline void glCullFaceError(GLenum  mode, const char *filename, int lineno)
{
  glCullFace(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCullFace]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCullFace(mode) glCullFaceError(mode, __FILE__, __LINE__)

inline void glDeleteListsError(GLuint  list, GLsizei  range, const char *filename, int lineno)
{
  glDeleteLists(list, range);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDeleteLists]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDeleteLists(list, range) glDeleteListsError(list, range, __FILE__, __LINE__)

inline void glDepthFuncError(GLenum  func, const char *filename, int lineno)
{
  glDepthFunc(func);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDepthFunc]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDepthFunc(func) glDepthFuncError(func, __FILE__, __LINE__)

inline void glDepthMaskError(GLboolean  flag, const char *filename, int lineno)
{
  glDepthMask(flag);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDepthMask]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDepthMask(flag) glDepthMaskError(flag, __FILE__, __LINE__)

inline void glDepthRangeError(GLclampd  zNear, GLclampd  zFar, const char *filename, int lineno)
{
  glDepthRange(zNear, zFar);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDepthRange]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDepthRange(zNear, zFar) glDepthRangeError(zNear, zFar, __FILE__, __LINE__)

inline void glDisableError(GLenum  cap, const char *filename, int lineno)
{
  glDisable(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDisable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDisable(cap) glDisableError(cap, __FILE__, __LINE__)

inline void glDisableClientStateError(GLenum  cap, const char *filename, int lineno)
{
  glDisableClientState(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDisableClientState]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDisableClientState(cap) glDisableClientStateError(cap, __FILE__, __LINE__)

inline void glDrawArraysError(GLenum  mode, GLint  first, GLsizei  count, const char *filename, int lineno)
{
  glDrawArrays(mode, first, count);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawArrays]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawArrays(mode, first, count) glDrawArraysError(mode, first, count, __FILE__, __LINE__)

inline void glDrawBufferError(GLenum  mode, const char *filename, int lineno)
{
  glDrawBuffer(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawBuffer(mode) glDrawBufferError(mode, __FILE__, __LINE__)

inline void glDrawElementsError(GLenum  mode, GLsizei  count, GLenum  type, const  GLvoid * indices, const char *filename, int lineno)
{
  glDrawElements(mode, count, type, indices);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawElements]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawElements(mode, count, type, indices) glDrawElementsError(mode, count, type, indices, __FILE__, __LINE__)

inline void glDrawRangeElementsError(GLenum  mode, GLuint  start, GLuint  end, GLsizei  count, GLenum  type, const  GLvoid * indices, const char *filename, int lineno)
{
  glDrawRangeElements(mode, start, end, count, type, indices);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawRangeElements]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawRangeElements(mode, start, end, count, type, indices) glDrawRangeElementsError(mode, start, end, count, type, indices, __FILE__, __LINE__)

inline void glDrawPixelsError(GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glDrawPixels(width, height, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawPixels]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawPixels(width, height, format, type, pixels) glDrawPixelsError(width, height, format, type, pixels, __FILE__, __LINE__)

inline void glEdgeFlagError(GLboolean  flag, const char *filename, int lineno)
{
  glEdgeFlag(flag);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEdgeFlag]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEdgeFlag(flag) glEdgeFlagError(flag, __FILE__, __LINE__)

inline void glEdgeFlagPointerError(GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glEdgeFlagPointer(stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEdgeFlagPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEdgeFlagPointer(stride, pointer) glEdgeFlagPointerError(stride, pointer, __FILE__, __LINE__)

inline void glEdgeFlagvError(const  GLboolean * flag, const char *filename, int lineno)
{
  glEdgeFlagv(flag);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEdgeFlagv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEdgeFlagv(flag) glEdgeFlagvError(flag, __FILE__, __LINE__)

inline void glEnableError(GLenum  cap, const char *filename, int lineno)
{
  glEnable(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEnable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEnable(cap) glEnableError(cap, __FILE__, __LINE__)

inline void glEnableClientStateError(GLenum  cap, const char *filename, int lineno)
{
  glEnableClientState(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEnableClientState]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEnableClientState(cap) glEnableClientStateError(cap, __FILE__, __LINE__)

inline void glEndError(const char *filename, int lineno)
{
  glEnd();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEnd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEnd() glEndError(__FILE__, __LINE__)

inline void glEndListError(const char *filename, int lineno)
{
  glEndList();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEndList]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEndList() glEndListError(__FILE__, __LINE__)

inline void glEvalCoord1dError(GLdouble  u, const char *filename, int lineno)
{
  glEvalCoord1d(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord1d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord1d(u) glEvalCoord1dError(u, __FILE__, __LINE__)

inline void glEvalCoord1dvError(const  GLdouble * u, const char *filename, int lineno)
{
  glEvalCoord1dv(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord1dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord1dv(u) glEvalCoord1dvError(u, __FILE__, __LINE__)

inline void glEvalCoord1fError(GLfloat  u, const char *filename, int lineno)
{
  glEvalCoord1f(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord1f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord1f(u) glEvalCoord1fError(u, __FILE__, __LINE__)

inline void glEvalCoord1fvError(const  GLfloat * u, const char *filename, int lineno)
{
  glEvalCoord1fv(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord1fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord1fv(u) glEvalCoord1fvError(u, __FILE__, __LINE__)

inline void glEvalCoord2dError(GLdouble  u, GLdouble  v, const char *filename, int lineno)
{
  glEvalCoord2d(u, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord2d(u, v) glEvalCoord2dError(u, v, __FILE__, __LINE__)

inline void glEvalCoord2dvError(const  GLdouble * u, const char *filename, int lineno)
{
  glEvalCoord2dv(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord2dv(u) glEvalCoord2dvError(u, __FILE__, __LINE__)

inline void glEvalCoord2fError(GLfloat  u, GLfloat  v, const char *filename, int lineno)
{
  glEvalCoord2f(u, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord2f(u, v) glEvalCoord2fError(u, v, __FILE__, __LINE__)

inline void glEvalCoord2fvError(const  GLfloat * u, const char *filename, int lineno)
{
  glEvalCoord2fv(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord2fv(u) glEvalCoord2fvError(u, __FILE__, __LINE__)

inline void glEvalMesh1Error(GLenum  mode, GLint  i1, GLint  i2, const char *filename, int lineno)
{
  glEvalMesh1(mode, i1, i2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalMesh1]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalMesh1(mode, i1, i2) glEvalMesh1Error(mode, i1, i2, __FILE__, __LINE__)

inline void glEvalMesh2Error(GLenum  mode, GLint  i1, GLint  i2, GLint  j1, GLint  j2, const char *filename, int lineno)
{
  glEvalMesh2(mode, i1, i2, j1, j2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalMesh2]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalMesh2(mode, i1, i2, j1, j2) glEvalMesh2Error(mode, i1, i2, j1, j2, __FILE__, __LINE__)

inline void glEvalPoint1Error(GLint  i, const char *filename, int lineno)
{
  glEvalPoint1(i);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalPoint1]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalPoint1(i) glEvalPoint1Error(i, __FILE__, __LINE__)

inline void glEvalPoint2Error(GLint  i, GLint  j, const char *filename, int lineno)
{
  glEvalPoint2(i, j);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalPoint2]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalPoint2(i, j) glEvalPoint2Error(i, j, __FILE__, __LINE__)

inline void glFeedbackBufferError(GLsizei  size, GLenum  type, GLfloat * buffer, const char *filename, int lineno)
{
  glFeedbackBuffer(size, type, buffer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFeedbackBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFeedbackBuffer(size, type, buffer) glFeedbackBufferError(size, type, buffer, __FILE__, __LINE__)

inline void glFinishError(const char *filename, int lineno)
{
  glFinish();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFinish]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFinish() glFinishError(__FILE__, __LINE__)

inline void glFlushError(const char *filename, int lineno)
{
  glFlush();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFlush]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFlush() glFlushError(__FILE__, __LINE__)

inline void glFogfError(GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glFogf(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogf(pname, param) glFogfError(pname, param, __FILE__, __LINE__)

inline void glFogfvError(GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glFogfv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogfv(pname, params) glFogfvError(pname, params, __FILE__, __LINE__)

inline void glFogiError(GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glFogi(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogi]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogi(pname, param) glFogiError(pname, param, __FILE__, __LINE__)

inline void glFogivError(GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glFogiv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogiv(pname, params) glFogivError(pname, params, __FILE__, __LINE__)

inline void glFrontFaceError(GLenum  mode, const char *filename, int lineno)
{
  glFrontFace(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFrontFace]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFrontFace(mode) glFrontFaceError(mode, __FILE__, __LINE__)

inline void glFrustumError(GLdouble  left, GLdouble  right, GLdouble  bottom, GLdouble  top, GLdouble  zNear, GLdouble  zFar, const char *filename, int lineno)
{
  glFrustum(left, right, bottom, top, zNear, zFar);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFrustum]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFrustum(left, right, bottom, top, zNear, zFar) glFrustumError(left, right, bottom, top, zNear, zFar, __FILE__, __LINE__)

inline GLuint glGenListsError(GLsizei  range, const char *filename, int lineno)
{
  GLuint rv;
  rv = glGenLists(range);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGenLists]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glGenLists(range) glGenListsError(range, __FILE__, __LINE__)

inline void glGetBooleanvError(GLenum  pname, GLboolean * params, const char *filename, int lineno)
{
  glGetBooleanv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetBooleanv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetBooleanv(pname, params) glGetBooleanvError(pname, params, __FILE__, __LINE__)

inline void glGetClipPlaneError(GLenum  plane, GLdouble * equation, const char *filename, int lineno)
{
  glGetClipPlane(plane, equation);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetClipPlane]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetClipPlane(plane, equation) glGetClipPlaneError(plane, equation, __FILE__, __LINE__)

inline void glGetColorTableError(GLenum  target, GLenum  format, GLenum  type, GLvoid * table, const char *filename, int lineno)
{
  glGetColorTable(target, format, type, table);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetColorTable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetColorTable(target, format, type, table) glGetColorTableError(target, format, type, table, __FILE__, __LINE__)

inline void glGetColorTableParameterfvError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetColorTableParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetColorTableParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetColorTableParameterfv(target, pname, params) glGetColorTableParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glGetColorTableParameterivError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetColorTableParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetColorTableParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetColorTableParameteriv(target, pname, params) glGetColorTableParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glGetConvolutionFilterError(GLenum  target, GLenum  format, GLenum  type, GLvoid * image, const char *filename, int lineno)
{
  glGetConvolutionFilter(target, format, type, image);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetConvolutionFilter]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetConvolutionFilter(target, format, type, image) glGetConvolutionFilterError(target, format, type, image, __FILE__, __LINE__)

inline void glGetConvolutionParameterfvError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetConvolutionParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetConvolutionParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetConvolutionParameterfv(target, pname, params) glGetConvolutionParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glGetConvolutionParameterivError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetConvolutionParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetConvolutionParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetConvolutionParameteriv(target, pname, params) glGetConvolutionParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glGetDoublevError(GLenum  pname, GLdouble * params, const char *filename, int lineno)
{
  glGetDoublev(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetDoublev]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetDoublev(pname, params) glGetDoublevError(pname, params, __FILE__, __LINE__)

inline GLenum glGetErrorError(const char *filename, int lineno)
{
  GLenum rv;
  rv = glGetError();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetError]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glGetError() glGetErrorError(__FILE__, __LINE__)

inline void glGetFloatvError(GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetFloatv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetFloatv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetFloatv(pname, params) glGetFloatvError(pname, params, __FILE__, __LINE__)

inline void glGetHistogramError(GLenum  target, GLboolean  reset, GLenum  format, GLenum  type, GLvoid * values, const char *filename, int lineno)
{
  glGetHistogram(target, reset, format, type, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetHistogram]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetHistogram(target, reset, format, type, values) glGetHistogramError(target, reset, format, type, values, __FILE__, __LINE__)

inline void glGetHistogramParameterfvError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetHistogramParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetHistogramParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetHistogramParameterfv(target, pname, params) glGetHistogramParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glGetHistogramParameterivError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetHistogramParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetHistogramParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetHistogramParameteriv(target, pname, params) glGetHistogramParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glGetIntegervError(GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetIntegerv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetIntegerv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetIntegerv(pname, params) glGetIntegervError(pname, params, __FILE__, __LINE__)

inline void glGetLightfvError(GLenum  light, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetLightfv(light, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetLightfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetLightfv(light, pname, params) glGetLightfvError(light, pname, params, __FILE__, __LINE__)

inline void glGetLightivError(GLenum  light, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetLightiv(light, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetLightiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetLightiv(light, pname, params) glGetLightivError(light, pname, params, __FILE__, __LINE__)

inline void glGetMapdvError(GLenum  target, GLenum  query, GLdouble * v, const char *filename, int lineno)
{
  glGetMapdv(target, query, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMapdv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMapdv(target, query, v) glGetMapdvError(target, query, v, __FILE__, __LINE__)

inline void glGetMapfvError(GLenum  target, GLenum  query, GLfloat * v, const char *filename, int lineno)
{
  glGetMapfv(target, query, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMapfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMapfv(target, query, v) glGetMapfvError(target, query, v, __FILE__, __LINE__)

inline void glGetMapivError(GLenum  target, GLenum  query, GLint * v, const char *filename, int lineno)
{
  glGetMapiv(target, query, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMapiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMapiv(target, query, v) glGetMapivError(target, query, v, __FILE__, __LINE__)

inline void glGetMaterialfvError(GLenum  face, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetMaterialfv(face, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMaterialfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMaterialfv(face, pname, params) glGetMaterialfvError(face, pname, params, __FILE__, __LINE__)

inline void glGetMaterialivError(GLenum  face, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetMaterialiv(face, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMaterialiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMaterialiv(face, pname, params) glGetMaterialivError(face, pname, params, __FILE__, __LINE__)

inline void glGetMinmaxError(GLenum  target, GLboolean  reset, GLenum  format, GLenum  type, GLvoid * values, const char *filename, int lineno)
{
  glGetMinmax(target, reset, format, type, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMinmax]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMinmax(target, reset, format, type, values) glGetMinmaxError(target, reset, format, type, values, __FILE__, __LINE__)

inline void glGetMinmaxParameterfvError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetMinmaxParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMinmaxParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMinmaxParameterfv(target, pname, params) glGetMinmaxParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glGetMinmaxParameterivError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetMinmaxParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMinmaxParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMinmaxParameteriv(target, pname, params) glGetMinmaxParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glGetPixelMapfvError(GLenum  map, GLfloat * values, const char *filename, int lineno)
{
  glGetPixelMapfv(map, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPixelMapfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPixelMapfv(map, values) glGetPixelMapfvError(map, values, __FILE__, __LINE__)

inline void glGetPixelMapuivError(GLenum  map, GLuint * values, const char *filename, int lineno)
{
  glGetPixelMapuiv(map, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPixelMapuiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPixelMapuiv(map, values) glGetPixelMapuivError(map, values, __FILE__, __LINE__)

inline void glGetPixelMapusvError(GLenum  map, GLushort * values, const char *filename, int lineno)
{
  glGetPixelMapusv(map, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPixelMapusv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPixelMapusv(map, values) glGetPixelMapusvError(map, values, __FILE__, __LINE__)

inline void glGetPointervError(GLenum  pname, GLvoid* * params, const char *filename, int lineno)
{
  glGetPointerv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPointerv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPointerv(pname, params) glGetPointervError(pname, params, __FILE__, __LINE__)

inline void glGetPolygonStippleError(GLubyte * mask, const char *filename, int lineno)
{
  glGetPolygonStipple(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPolygonStipple]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPolygonStipple(mask) glGetPolygonStippleError(mask, __FILE__, __LINE__)

inline void glGetSeparableFilterError(GLenum  target, GLenum  format, GLenum  type, GLvoid * row, GLvoid * column, GLvoid * span, const char *filename, int lineno)
{
  glGetSeparableFilter(target, format, type, row, column, span);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetSeparableFilter]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetSeparableFilter(target, format, type, row, column, span) glGetSeparableFilterError(target, format, type, row, column, span, __FILE__, __LINE__)

inline const GLubyte * glGetStringError(GLenum  name, const char *filename, int lineno)
{
  const GLubyte * rv;
  rv = glGetString(name);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetString]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glGetString(name) glGetStringError(name, __FILE__, __LINE__)

inline void glGetTexEnvfvError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetTexEnvfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexEnvfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexEnvfv(target, pname, params) glGetTexEnvfvError(target, pname, params, __FILE__, __LINE__)

inline void glGetTexEnvivError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetTexEnviv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexEnviv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexEnviv(target, pname, params) glGetTexEnvivError(target, pname, params, __FILE__, __LINE__)

inline void glGetTexGendvError(GLenum  coord, GLenum  pname, GLdouble * params, const char *filename, int lineno)
{
  glGetTexGendv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexGendv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexGendv(coord, pname, params) glGetTexGendvError(coord, pname, params, __FILE__, __LINE__)

inline void glGetTexGenfvError(GLenum  coord, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetTexGenfv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexGenfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexGenfv(coord, pname, params) glGetTexGenfvError(coord, pname, params, __FILE__, __LINE__)

inline void glGetTexGenivError(GLenum  coord, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetTexGeniv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexGeniv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexGeniv(coord, pname, params) glGetTexGenivError(coord, pname, params, __FILE__, __LINE__)

inline void glGetTexImageError(GLenum  target, GLint  level, GLenum  format, GLenum  type, GLvoid * pixels, const char *filename, int lineno)
{
  glGetTexImage(target, level, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexImage]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexImage(target, level, format, type, pixels) glGetTexImageError(target, level, format, type, pixels, __FILE__, __LINE__)

inline void glGetCompressedTexImageError(GLenum  target, GLint  lod, GLvoid * img, const char *filename, int lineno)
{
  glGetCompressedTexImage(target, lod, img);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetCompressedTexImage]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetCompressedTexImage(target, lod, img) glGetCompressedTexImageError(target, lod, img, __FILE__, __LINE__)

inline void glGetTexLevelParameterfvError(GLenum  target, GLint  level, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetTexLevelParameterfv(target, level, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexLevelParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexLevelParameterfv(target, level, pname, params) glGetTexLevelParameterfvError(target, level, pname, params, __FILE__, __LINE__)

inline void glGetTexLevelParameterivError(GLenum  target, GLint  level, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetTexLevelParameteriv(target, level, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexLevelParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexLevelParameteriv(target, level, pname, params) glGetTexLevelParameterivError(target, level, pname, params, __FILE__, __LINE__)

inline void glGetTexParameterfvError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetTexParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexParameterfv(target, pname, params) glGetTexParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glGetTexParameterivError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetTexParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexParameteriv(target, pname, params) glGetTexParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glHintError(GLenum  target, GLenum  mode, const char *filename, int lineno)
{
  glHint(target, mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glHint]: %s\n", filename, lineno, gluErrorString(error));
}
#define glHint(target, mode) glHintError(target, mode, __FILE__, __LINE__)

inline void glHistogramError(GLenum  target, GLsizei  width, GLenum  internalformat, GLboolean  sink, const char *filename, int lineno)
{
  glHistogram(target, width, internalformat, sink);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glHistogram]: %s\n", filename, lineno, gluErrorString(error));
}
#define glHistogram(target, width, internalformat, sink) glHistogramError(target, width, internalformat, sink, __FILE__, __LINE__)

inline void glIndexMaskError(GLuint  mask, const char *filename, int lineno)
{
  glIndexMask(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexMask]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexMask(mask) glIndexMaskError(mask, __FILE__, __LINE__)

inline void glIndexPointerError(GLenum  type, GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glIndexPointer(type, stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexPointer(type, stride, pointer) glIndexPointerError(type, stride, pointer, __FILE__, __LINE__)

inline void glInitNamesError(const char *filename, int lineno)
{
  glInitNames();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glInitNames]: %s\n", filename, lineno, gluErrorString(error));
}
#define glInitNames() glInitNamesError(__FILE__, __LINE__)

inline void glInterleavedArraysError(GLenum  format, GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glInterleavedArrays(format, stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glInterleavedArrays]: %s\n", filename, lineno, gluErrorString(error));
}
#define glInterleavedArrays(format, stride, pointer) glInterleavedArraysError(format, stride, pointer, __FILE__, __LINE__)

inline GLboolean glIsEnabledError(GLenum  cap, const char *filename, int lineno)
{
  GLboolean rv;
  rv = glIsEnabled(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIsEnabled]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glIsEnabled(cap) glIsEnabledError(cap, __FILE__, __LINE__)

inline GLboolean glIsListError(GLuint  list, const char *filename, int lineno)
{
  GLboolean rv;
  rv = glIsList(list);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIsList]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glIsList(list) glIsListError(list, __FILE__, __LINE__)

inline GLboolean glIsTextureError(GLuint  texture, const char *filename, int lineno)
{
  GLboolean rv;
  rv = glIsTexture(texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIsTexture]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glIsTexture(texture) glIsTextureError(texture, __FILE__, __LINE__)

inline void glLightModelfError(GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glLightModelf(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModelf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModelf(pname, param) glLightModelfError(pname, param, __FILE__, __LINE__)

inline void glLightModelfvError(GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glLightModelfv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModelfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModelfv(pname, params) glLightModelfvError(pname, params, __FILE__, __LINE__)

inline void glLightModeliError(GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glLightModeli(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModeli]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModeli(pname, param) glLightModeliError(pname, param, __FILE__, __LINE__)

inline void glLightModelivError(GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glLightModeliv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModeliv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModeliv(pname, params) glLightModelivError(pname, params, __FILE__, __LINE__)

inline void glLightfError(GLenum  light, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glLightf(light, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightf(light, pname, param) glLightfError(light, pname, param, __FILE__, __LINE__)

inline void glLightfvError(GLenum  light, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glLightfv(light, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightfv(light, pname, params) glLightfvError(light, pname, params, __FILE__, __LINE__)

inline void glLightiError(GLenum  light, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glLighti(light, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLighti]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLighti(light, pname, param) glLightiError(light, pname, param, __FILE__, __LINE__)

inline void glLightivError(GLenum  light, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glLightiv(light, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightiv(light, pname, params) glLightivError(light, pname, params, __FILE__, __LINE__)

inline void glLineStippleError(GLint  factor, GLushort  pattern, const char *filename, int lineno)
{
  glLineStipple(factor, pattern);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLineStipple]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLineStipple(factor, pattern) glLineStippleError(factor, pattern, __FILE__, __LINE__)

inline void glLineWidthError(GLfloat  width, const char *filename, int lineno)
{
  glLineWidth(width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLineWidth]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLineWidth(width) glLineWidthError(width, __FILE__, __LINE__)

inline void glListBaseError(GLuint  base, const char *filename, int lineno)
{
  glListBase(base);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glListBase]: %s\n", filename, lineno, gluErrorString(error));
}
#define glListBase(base) glListBaseError(base, __FILE__, __LINE__)

inline void glLoadIdentityError(const char *filename, int lineno)
{
  glLoadIdentity();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadIdentity]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadIdentity() glLoadIdentityError(__FILE__, __LINE__)

inline void glLoadMatrixdError(const  GLdouble * m, const char *filename, int lineno)
{
  glLoadMatrixd(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadMatrixd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadMatrixd(m) glLoadMatrixdError(m, __FILE__, __LINE__)

inline void glLoadMatrixfError(const  GLfloat * m, const char *filename, int lineno)
{
  glLoadMatrixf(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadMatrixf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadMatrixf(m) glLoadMatrixfError(m, __FILE__, __LINE__)

inline void glLoadTransposeMatrixdError(const  GLdouble * m, const char *filename, int lineno)
{
  glLoadTransposeMatrixd(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadTransposeMatrixd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadTransposeMatrixd(m) glLoadTransposeMatrixdError(m, __FILE__, __LINE__)

inline void glLoadTransposeMatrixfError(const  GLfloat * m, const char *filename, int lineno)
{
  glLoadTransposeMatrixf(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadTransposeMatrixf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadTransposeMatrixf(m) glLoadTransposeMatrixfError(m, __FILE__, __LINE__)

inline void glLoadNameError(GLuint  name, const char *filename, int lineno)
{
  glLoadName(name);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadName]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadName(name) glLoadNameError(name, __FILE__, __LINE__)

inline void glLogicOpError(GLenum  opcode, const char *filename, int lineno)
{
  glLogicOp(opcode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLogicOp]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLogicOp(opcode) glLogicOpError(opcode, __FILE__, __LINE__)

inline void glMap1dError(GLenum  target, GLdouble  u1, GLdouble  u2, GLint  stride, GLint  order, const  GLdouble * points, const char *filename, int lineno)
{
  glMap1d(target, u1, u2, stride, order, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMap1d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMap1d(target, u1, u2, stride, order, points) glMap1dError(target, u1, u2, stride, order, points, __FILE__, __LINE__)

inline void glMap1fError(GLenum  target, GLfloat  u1, GLfloat  u2, GLint  stride, GLint  order, const  GLfloat * points, const char *filename, int lineno)
{
  glMap1f(target, u1, u2, stride, order, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMap1f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMap1f(target, u1, u2, stride, order, points) glMap1fError(target, u1, u2, stride, order, points, __FILE__, __LINE__)

inline void glMap2dError(GLenum  target, GLdouble  u1, GLdouble  u2, GLint  ustride, GLint  uorder, GLdouble  v1, GLdouble  v2, GLint  vstride, GLint  vorder, const  GLdouble * points, const char *filename, int lineno)
{
  glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMap2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) glMap2dError(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points, __FILE__, __LINE__)

inline void glMap2fError(GLenum  target, GLfloat  u1, GLfloat  u2, GLint  ustride, GLint  uorder, GLfloat  v1, GLfloat  v2, GLint  vstride, GLint  vorder, const  GLfloat * points, const char *filename, int lineno)
{
  glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMap2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) glMap2fError(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points, __FILE__, __LINE__)

inline void glMapGrid1dError(GLint  un, GLdouble  u1, GLdouble  u2, const char *filename, int lineno)
{
  glMapGrid1d(un, u1, u2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMapGrid1d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMapGrid1d(un, u1, u2) glMapGrid1dError(un, u1, u2, __FILE__, __LINE__)

inline void glMapGrid1fError(GLint  un, GLfloat  u1, GLfloat  u2, const char *filename, int lineno)
{
  glMapGrid1f(un, u1, u2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMapGrid1f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMapGrid1f(un, u1, u2) glMapGrid1fError(un, u1, u2, __FILE__, __LINE__)

inline void glMapGrid2dError(GLint  un, GLdouble  u1, GLdouble  u2, GLint  vn, GLdouble  v1, GLdouble  v2, const char *filename, int lineno)
{
  glMapGrid2d(un, u1, u2, vn, v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMapGrid2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMapGrid2d(un, u1, u2, vn, v1, v2) glMapGrid2dError(un, u1, u2, vn, v1, v2, __FILE__, __LINE__)

inline void glMapGrid2fError(GLint  un, GLfloat  u1, GLfloat  u2, GLint  vn, GLfloat  v1, GLfloat  v2, const char *filename, int lineno)
{
  glMapGrid2f(un, u1, u2, vn, v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMapGrid2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMapGrid2f(un, u1, u2, vn, v1, v2) glMapGrid2fError(un, u1, u2, vn, v1, v2, __FILE__, __LINE__)

inline void glMaterialfError(GLenum  face, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glMaterialf(face, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMaterialf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMaterialf(face, pname, param) glMaterialfError(face, pname, param, __FILE__, __LINE__)

inline void glMaterialfvError(GLenum  face, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glMaterialfv(face, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMaterialfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMaterialfv(face, pname, params) glMaterialfvError(face, pname, params, __FILE__, __LINE__)

inline void glMaterialiError(GLenum  face, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glMateriali(face, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMateriali]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMateriali(face, pname, param) glMaterialiError(face, pname, param, __FILE__, __LINE__)

inline void glMaterialivError(GLenum  face, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glMaterialiv(face, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMaterialiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMaterialiv(face, pname, params) glMaterialivError(face, pname, params, __FILE__, __LINE__)

inline void glMatrixModeError(GLenum  mode, const char *filename, int lineno)
{
  glMatrixMode(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMatrixMode]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMatrixMode(mode) glMatrixModeError(mode, __FILE__, __LINE__)

inline void glMinmaxError(GLenum  target, GLenum  internalformat, GLboolean  sink, const char *filename, int lineno)
{
  glMinmax(target, internalformat, sink);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMinmax]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMinmax(target, internalformat, sink) glMinmaxError(target, internalformat, sink, __FILE__, __LINE__)

inline void glMultiTexCoord1dError(GLenum  texture, GLdouble  s, const char *filename, int lineno)
{
  glMultiTexCoord1d(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1d(texture, s) glMultiTexCoord1dError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1dvError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord1dv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1dv(texture, v) glMultiTexCoord1dvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord1fError(GLenum  texture, GLfloat  s, const char *filename, int lineno)
{
  glMultiTexCoord1f(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1f(texture, s) glMultiTexCoord1fError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1fvError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord1fv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1fv(texture, v) glMultiTexCoord1fvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord1iError(GLenum  texture, GLint  s, const char *filename, int lineno)
{
  glMultiTexCoord1i(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1i(texture, s) glMultiTexCoord1iError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1ivError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord1iv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1iv(texture, v) glMultiTexCoord1ivError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord1sError(GLenum  texture, GLshort  s, const char *filename, int lineno)
{
  glMultiTexCoord1s(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1s(texture, s) glMultiTexCoord1sError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1svError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord1sv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1sv(texture, v) glMultiTexCoord1svError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2dError(GLenum  texture, GLdouble  s, GLdouble  t, const char *filename, int lineno)
{
  glMultiTexCoord2d(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2d(texture, s, t) glMultiTexCoord2dError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2dvError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord2dv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2dv(texture, v) glMultiTexCoord2dvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2fError(GLenum  texture, GLfloat  s, GLfloat  t, const char *filename, int lineno)
{
  glMultiTexCoord2f(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2f(texture, s, t) glMultiTexCoord2fError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2fvError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord2fv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2fv(texture, v) glMultiTexCoord2fvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2iError(GLenum  texture, GLint  s, GLint  t, const char *filename, int lineno)
{
  glMultiTexCoord2i(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2i(texture, s, t) glMultiTexCoord2iError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2ivError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord2iv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2iv(texture, v) glMultiTexCoord2ivError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2sError(GLenum  texture, GLshort  s, GLshort  t, const char *filename, int lineno)
{
  glMultiTexCoord2s(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2s(texture, s, t) glMultiTexCoord2sError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2svError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord2sv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2sv(texture, v) glMultiTexCoord2svError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3dError(GLenum  texture, GLdouble  s, GLdouble  t, GLdouble  r, const char *filename, int lineno)
{
  glMultiTexCoord3d(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3d(texture, s, t, r) glMultiTexCoord3dError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3dvError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord3dv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3dv(texture, v) glMultiTexCoord3dvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3fError(GLenum  texture, GLfloat  s, GLfloat  t, GLfloat  r, const char *filename, int lineno)
{
  glMultiTexCoord3f(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3f(texture, s, t, r) glMultiTexCoord3fError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3fvError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord3fv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3fv(texture, v) glMultiTexCoord3fvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3iError(GLenum  texture, GLint  s, GLint  t, GLint  r, const char *filename, int lineno)
{
  glMultiTexCoord3i(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3i(texture, s, t, r) glMultiTexCoord3iError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3ivError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord3iv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3iv(texture, v) glMultiTexCoord3ivError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3sError(GLenum  texture, GLshort  s, GLshort  t, GLshort  r, const char *filename, int lineno)
{
  glMultiTexCoord3s(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3s(texture, s, t, r) glMultiTexCoord3sError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3svError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord3sv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3sv(texture, v) glMultiTexCoord3svError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4dError(GLenum  texture, GLdouble  s, GLdouble  t, GLdouble  r, GLdouble  q, const char *filename, int lineno)
{
  glMultiTexCoord4d(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4d(texture, s, t, r, q) glMultiTexCoord4dError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4dvError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord4dv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4dv(texture, v) glMultiTexCoord4dvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4fError(GLenum  texture, GLfloat  s, GLfloat  t, GLfloat  r, GLfloat  q, const char *filename, int lineno)
{
  glMultiTexCoord4f(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4f(texture, s, t, r, q) glMultiTexCoord4fError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4fvError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord4fv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4fv(texture, v) glMultiTexCoord4fvError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4iError(GLenum  texture, GLint  s, GLint  t, GLint  r, GLint  q, const char *filename, int lineno)
{
  glMultiTexCoord4i(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4i(texture, s, t, r, q) glMultiTexCoord4iError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4ivError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord4iv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4iv(texture, v) glMultiTexCoord4ivError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4sError(GLenum  texture, GLshort  s, GLshort  t, GLshort  r, GLshort  q, const char *filename, int lineno)
{
  glMultiTexCoord4s(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4s(texture, s, t, r, q) glMultiTexCoord4sError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4svError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord4sv(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4sv(texture, v) glMultiTexCoord4svError(texture, v, __FILE__, __LINE__)

inline void glMultMatrixdError(const  GLdouble * m, const char *filename, int lineno)
{
  glMultMatrixd(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultMatrixd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultMatrixd(m) glMultMatrixdError(m, __FILE__, __LINE__)

inline void glMultMatrixfError(const  GLfloat * m, const char *filename, int lineno)
{
  glMultMatrixf(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultMatrixf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultMatrixf(m) glMultMatrixfError(m, __FILE__, __LINE__)

inline void glMultTransposeMatrixdError(const  GLdouble * m, const char *filename, int lineno)
{
  glMultTransposeMatrixd(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultTransposeMatrixd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultTransposeMatrixd(m) glMultTransposeMatrixdError(m, __FILE__, __LINE__)

inline void glMultTransposeMatrixfError(const  GLfloat * m, const char *filename, int lineno)
{
  glMultTransposeMatrixf(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultTransposeMatrixf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultTransposeMatrixf(m) glMultTransposeMatrixfError(m, __FILE__, __LINE__)

inline void glNewListError(GLuint  list, GLenum  mode, const char *filename, int lineno)
{
  glNewList(list, mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNewList]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNewList(list, mode) glNewListError(list, mode, __FILE__, __LINE__)

inline void glNormalPointerError(GLenum  type, GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glNormalPointer(type, stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormalPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormalPointer(type, stride, pointer) glNormalPointerError(type, stride, pointer, __FILE__, __LINE__)

inline void glOrthoError(GLdouble  left, GLdouble  right, GLdouble  bottom, GLdouble  top, GLdouble  zNear, GLdouble  zFar, const char *filename, int lineno)
{
  glOrtho(left, right, bottom, top, zNear, zFar);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glOrtho]: %s\n", filename, lineno, gluErrorString(error));
}
#define glOrtho(left, right, bottom, top, zNear, zFar) glOrthoError(left, right, bottom, top, zNear, zFar, __FILE__, __LINE__)

inline void glPassThroughError(GLfloat  token, const char *filename, int lineno)
{
  glPassThrough(token);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPassThrough]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPassThrough(token) glPassThroughError(token, __FILE__, __LINE__)

inline void glPixelMapfvError(GLenum  map, GLsizei  mapsize, const  GLfloat * values, const char *filename, int lineno)
{
  glPixelMapfv(map, mapsize, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelMapfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelMapfv(map, mapsize, values) glPixelMapfvError(map, mapsize, values, __FILE__, __LINE__)

inline void glPixelMapuivError(GLenum  map, GLsizei  mapsize, const  GLuint * values, const char *filename, int lineno)
{
  glPixelMapuiv(map, mapsize, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelMapuiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelMapuiv(map, mapsize, values) glPixelMapuivError(map, mapsize, values, __FILE__, __LINE__)

inline void glPixelMapusvError(GLenum  map, GLsizei  mapsize, const  GLushort * values, const char *filename, int lineno)
{
  glPixelMapusv(map, mapsize, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelMapusv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelMapusv(map, mapsize, values) glPixelMapusvError(map, mapsize, values, __FILE__, __LINE__)

inline void glPixelStorefError(GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glPixelStoref(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelStoref]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelStoref(pname, param) glPixelStorefError(pname, param, __FILE__, __LINE__)

inline void glPixelStoreiError(GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glPixelStorei(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelStorei]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelStorei(pname, param) glPixelStoreiError(pname, param, __FILE__, __LINE__)

inline void glPixelTransferfError(GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glPixelTransferf(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelTransferf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelTransferf(pname, param) glPixelTransferfError(pname, param, __FILE__, __LINE__)

inline void glPixelTransferiError(GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glPixelTransferi(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelTransferi]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelTransferi(pname, param) glPixelTransferiError(pname, param, __FILE__, __LINE__)

inline void glPixelZoomError(GLfloat  xfactor, GLfloat  yfactor, const char *filename, int lineno)
{
  glPixelZoom(xfactor, yfactor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelZoom]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelZoom(xfactor, yfactor) glPixelZoomError(xfactor, yfactor, __FILE__, __LINE__)

inline void glPointSizeError(GLfloat  size, const char *filename, int lineno)
{
  glPointSize(size);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPointSize]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPointSize(size) glPointSizeError(size, __FILE__, __LINE__)

inline void glPolygonModeError(GLenum  face, GLenum  mode, const char *filename, int lineno)
{
  glPolygonMode(face, mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPolygonMode]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPolygonMode(face, mode) glPolygonModeError(face, mode, __FILE__, __LINE__)

inline void glPolygonOffsetError(GLfloat  factor, GLfloat  units, const char *filename, int lineno)
{
  glPolygonOffset(factor, units);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPolygonOffset]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPolygonOffset(factor, units) glPolygonOffsetError(factor, units, __FILE__, __LINE__)

inline void glPolygonStippleError(const  GLubyte * mask, const char *filename, int lineno)
{
  glPolygonStipple(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPolygonStipple]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPolygonStipple(mask) glPolygonStippleError(mask, __FILE__, __LINE__)

inline void glPopAttribError(const char *filename, int lineno)
{
  glPopAttrib();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopAttrib() glPopAttribError(__FILE__, __LINE__)

inline void glPopClientAttribError(const char *filename, int lineno)
{
  glPopClientAttrib();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopClientAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopClientAttrib() glPopClientAttribError(__FILE__, __LINE__)

inline void glPopMatrixError(const char *filename, int lineno)
{
  glPopMatrix();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopMatrix]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopMatrix() glPopMatrixError(__FILE__, __LINE__)

inline void glPopNameError(const char *filename, int lineno)
{
  glPopName();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopName]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopName() glPopNameError(__FILE__, __LINE__)

inline void glBindTextureError(GLenum  target, GLuint  texture, const char *filename, int lineno)
{
  glBindTexture(target, texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBindTexture]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBindTexture(target, texture) glBindTextureError(target, texture, __FILE__, __LINE__)

inline void glDeleteTexturesError(GLsizei  n, const  GLuint * textures, const char *filename, int lineno)
{
  glDeleteTextures(n, textures);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDeleteTextures]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDeleteTextures(n, textures) glDeleteTexturesError(n, textures, __FILE__, __LINE__)

inline void glPrioritizeTexturesError(GLsizei  n, const  GLuint * textures, const  GLclampf * priorities, const char *filename, int lineno)
{
  glPrioritizeTextures(n, textures, priorities);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPrioritizeTextures]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPrioritizeTextures(n, textures, priorities) glPrioritizeTexturesError(n, textures, priorities, __FILE__, __LINE__)

inline void glGenTexturesError(GLsizei  n, GLuint * textures, const char *filename, int lineno)
{
  glGenTextures(n, textures);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGenTextures]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGenTextures(n, textures) glGenTexturesError(n, textures, __FILE__, __LINE__)

inline GLboolean glAreTexturesResidentError(GLsizei  n, const  GLuint * textures, GLboolean * residences, const char *filename, int lineno)
{
  GLboolean rv;
  rv = glAreTexturesResident(n, textures, residences);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glAreTexturesResident]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glAreTexturesResident(n, textures, residences) glAreTexturesResidentError(n, textures, residences, __FILE__, __LINE__)

inline void glPushAttribError(GLbitfield  mask, const char *filename, int lineno)
{
  glPushAttrib(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushAttrib(mask) glPushAttribError(mask, __FILE__, __LINE__)

inline void glPushClientAttribError(GLbitfield  mask, const char *filename, int lineno)
{
  glPushClientAttrib(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushClientAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushClientAttrib(mask) glPushClientAttribError(mask, __FILE__, __LINE__)

inline void glPushMatrixError(const char *filename, int lineno)
{
  glPushMatrix();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushMatrix]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushMatrix() glPushMatrixError(__FILE__, __LINE__)

inline void glPushNameError(GLuint  name, const char *filename, int lineno)
{
  glPushName(name);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushName]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushName(name) glPushNameError(name, __FILE__, __LINE__)

inline void glRasterPos2dError(GLdouble  x, GLdouble  y, const char *filename, int lineno)
{
  glRasterPos2d(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2d(x, y) glRasterPos2dError(x, y, __FILE__, __LINE__)

inline void glRasterPos2dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glRasterPos2dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2dv(v) glRasterPos2dvError(v, __FILE__, __LINE__)

inline void glRasterPos2fError(GLfloat  x, GLfloat  y, const char *filename, int lineno)
{
  glRasterPos2f(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2f(x, y) glRasterPos2fError(x, y, __FILE__, __LINE__)

inline void glRasterPos2fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glRasterPos2fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2fv(v) glRasterPos2fvError(v, __FILE__, __LINE__)

inline void glRasterPos2iError(GLint  x, GLint  y, const char *filename, int lineno)
{
  glRasterPos2i(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2i(x, y) glRasterPos2iError(x, y, __FILE__, __LINE__)

inline void glRasterPos2ivError(const  GLint * v, const char *filename, int lineno)
{
  glRasterPos2iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2iv(v) glRasterPos2ivError(v, __FILE__, __LINE__)

inline void glRasterPos2sError(GLshort  x, GLshort  y, const char *filename, int lineno)
{
  glRasterPos2s(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2s(x, y) glRasterPos2sError(x, y, __FILE__, __LINE__)

inline void glRasterPos2svError(const  GLshort * v, const char *filename, int lineno)
{
  glRasterPos2sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2sv(v) glRasterPos2svError(v, __FILE__, __LINE__)

inline void glRasterPos3dError(GLdouble  x, GLdouble  y, GLdouble  z, const char *filename, int lineno)
{
  glRasterPos3d(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3d(x, y, z) glRasterPos3dError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos3dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glRasterPos3dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3dv(v) glRasterPos3dvError(v, __FILE__, __LINE__)

inline void glRasterPos3fError(GLfloat  x, GLfloat  y, GLfloat  z, const char *filename, int lineno)
{
  glRasterPos3f(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3f(x, y, z) glRasterPos3fError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos3fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glRasterPos3fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3fv(v) glRasterPos3fvError(v, __FILE__, __LINE__)

inline void glRasterPos3iError(GLint  x, GLint  y, GLint  z, const char *filename, int lineno)
{
  glRasterPos3i(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3i(x, y, z) glRasterPos3iError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos3ivError(const  GLint * v, const char *filename, int lineno)
{
  glRasterPos3iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3iv(v) glRasterPos3ivError(v, __FILE__, __LINE__)

inline void glRasterPos3sError(GLshort  x, GLshort  y, GLshort  z, const char *filename, int lineno)
{
  glRasterPos3s(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3s(x, y, z) glRasterPos3sError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos3svError(const  GLshort * v, const char *filename, int lineno)
{
  glRasterPos3sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3sv(v) glRasterPos3svError(v, __FILE__, __LINE__)

inline void glRasterPos4dError(GLdouble  x, GLdouble  y, GLdouble  z, GLdouble  w, const char *filename, int lineno)
{
  glRasterPos4d(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4d(x, y, z, w) glRasterPos4dError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos4dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glRasterPos4dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4dv(v) glRasterPos4dvError(v, __FILE__, __LINE__)

inline void glRasterPos4fError(GLfloat  x, GLfloat  y, GLfloat  z, GLfloat  w, const char *filename, int lineno)
{
  glRasterPos4f(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4f(x, y, z, w) glRasterPos4fError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos4fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glRasterPos4fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4fv(v) glRasterPos4fvError(v, __FILE__, __LINE__)

inline void glRasterPos4iError(GLint  x, GLint  y, GLint  z, GLint  w, const char *filename, int lineno)
{
  glRasterPos4i(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4i(x, y, z, w) glRasterPos4iError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos4ivError(const  GLint * v, const char *filename, int lineno)
{
  glRasterPos4iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4iv(v) glRasterPos4ivError(v, __FILE__, __LINE__)

inline void glRasterPos4sError(GLshort  x, GLshort  y, GLshort  z, GLshort  w, const char *filename, int lineno)
{
  glRasterPos4s(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4s(x, y, z, w) glRasterPos4sError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos4svError(const  GLshort * v, const char *filename, int lineno)
{
  glRasterPos4sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4sv(v) glRasterPos4svError(v, __FILE__, __LINE__)

inline void glReadBufferError(GLenum  mode, const char *filename, int lineno)
{
  glReadBuffer(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glReadBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glReadBuffer(mode) glReadBufferError(mode, __FILE__, __LINE__)

inline void glReadPixelsError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, GLvoid * pixels, const char *filename, int lineno)
{
  glReadPixels(x, y, width, height, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glReadPixels]: %s\n", filename, lineno, gluErrorString(error));
}
#define glReadPixels(x, y, width, height, format, type, pixels) glReadPixelsError(x, y, width, height, format, type, pixels, __FILE__, __LINE__)

inline void glRectdError(GLdouble  x1, GLdouble  y1, GLdouble  x2, GLdouble  y2, const char *filename, int lineno)
{
  glRectd(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectd(x1, y1, x2, y2) glRectdError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectdvError(const  GLdouble * v1, const  GLdouble * v2, const char *filename, int lineno)
{
  glRectdv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectdv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectdv(v1, v2) glRectdvError(v1, v2, __FILE__, __LINE__)

inline void glRectfError(GLfloat  x1, GLfloat  y1, GLfloat  x2, GLfloat  y2, const char *filename, int lineno)
{
  glRectf(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectf(x1, y1, x2, y2) glRectfError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectfvError(const  GLfloat * v1, const  GLfloat * v2, const char *filename, int lineno)
{
  glRectfv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectfv(v1, v2) glRectfvError(v1, v2, __FILE__, __LINE__)

inline void glRectiError(GLint  x1, GLint  y1, GLint  x2, GLint  y2, const char *filename, int lineno)
{
  glRecti(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRecti]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRecti(x1, y1, x2, y2) glRectiError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectivError(const  GLint * v1, const  GLint * v2, const char *filename, int lineno)
{
  glRectiv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectiv(v1, v2) glRectivError(v1, v2, __FILE__, __LINE__)

inline void glRectsError(GLshort  x1, GLshort  y1, GLshort  x2, GLshort  y2, const char *filename, int lineno)
{
  glRects(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRects]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRects(x1, y1, x2, y2) glRectsError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectsvError(const  GLshort * v1, const  GLshort * v2, const char *filename, int lineno)
{
  glRectsv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectsv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectsv(v1, v2) glRectsvError(v1, v2, __FILE__, __LINE__)

inline GLint glRenderModeError(GLenum  mode, const char *filename, int lineno)
{
  GLint rv;
  rv = glRenderMode(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRenderMode]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glRenderMode(mode) glRenderModeError(mode, __FILE__, __LINE__)

inline void glResetHistogramError(GLenum  target, const char *filename, int lineno)
{
  glResetHistogram(target);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glResetHistogram]: %s\n", filename, lineno, gluErrorString(error));
}
#define glResetHistogram(target) glResetHistogramError(target, __FILE__, __LINE__)

inline void glResetMinmaxError(GLenum  target, const char *filename, int lineno)
{
  glResetMinmax(target);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glResetMinmax]: %s\n", filename, lineno, gluErrorString(error));
}
#define glResetMinmax(target) glResetMinmaxError(target, __FILE__, __LINE__)

inline void glRotatedError(GLdouble  angle, GLdouble  x, GLdouble  y, GLdouble  z, const char *filename, int lineno)
{
  glRotated(angle, x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRotated]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRotated(angle, x, y, z) glRotatedError(angle, x, y, z, __FILE__, __LINE__)

inline void glRotatefError(GLfloat  angle, GLfloat  x, GLfloat  y, GLfloat  z, const char *filename, int lineno)
{
  glRotatef(angle, x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRotatef]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRotatef(angle, x, y, z) glRotatefError(angle, x, y, z, __FILE__, __LINE__)

inline void glSampleCoverageError(GLclampf  value, GLboolean  invert, const char *filename, int lineno)
{
  glSampleCoverage(value, invert);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSampleCoverage]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSampleCoverage(value, invert) glSampleCoverageError(value, invert, __FILE__, __LINE__)

inline void glScaledError(GLdouble  x, GLdouble  y, GLdouble  z, const char *filename, int lineno)
{
  glScaled(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glScaled]: %s\n", filename, lineno, gluErrorString(error));
}
#define glScaled(x, y, z) glScaledError(x, y, z, __FILE__, __LINE__)

inline void glScalefError(GLfloat  x, GLfloat  y, GLfloat  z, const char *filename, int lineno)
{
  glScalef(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glScalef]: %s\n", filename, lineno, gluErrorString(error));
}
#define glScalef(x, y, z) glScalefError(x, y, z, __FILE__, __LINE__)

inline void glScissorError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glScissor(x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glScissor]: %s\n", filename, lineno, gluErrorString(error));
}
#define glScissor(x, y, width, height) glScissorError(x, y, width, height, __FILE__, __LINE__)

inline void glSelectBufferError(GLsizei  size, GLuint * buffer, const char *filename, int lineno)
{
  glSelectBuffer(size, buffer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSelectBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSelectBuffer(size, buffer) glSelectBufferError(size, buffer, __FILE__, __LINE__)

inline void glSeparableFilter2DError(GLenum  target, GLenum  internalformat, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, GLvoid * row, GLvoid * column, const char *filename, int lineno)
{
  glSeparableFilter2D(target, internalformat, width, height, format, type, row, column);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSeparableFilter2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSeparableFilter2D(target, internalformat, width, height, format, type, row, column) glSeparableFilter2DError(target, internalformat, width, height, format, type, row, column, __FILE__, __LINE__)

inline void glShadeModelError(GLenum  mode, const char *filename, int lineno)
{
  glShadeModel(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glShadeModel]: %s\n", filename, lineno, gluErrorString(error));
}
#define glShadeModel(mode) glShadeModelError(mode, __FILE__, __LINE__)

inline void glStencilFuncError(GLenum  func, GLint  ref, GLuint  mask, const char *filename, int lineno)
{
  glStencilFunc(func, ref, mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glStencilFunc]: %s\n", filename, lineno, gluErrorString(error));
}
#define glStencilFunc(func, ref, mask) glStencilFuncError(func, ref, mask, __FILE__, __LINE__)

inline void glStencilMaskError(GLuint  mask, const char *filename, int lineno)
{
  glStencilMask(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glStencilMask]: %s\n", filename, lineno, gluErrorString(error));
}
#define glStencilMask(mask) glStencilMaskError(mask, __FILE__, __LINE__)

inline void glStencilOpError(GLenum  fail, GLenum  zfail, GLenum  zpass, const char *filename, int lineno)
{
  glStencilOp(fail, zfail, zpass);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glStencilOp]: %s\n", filename, lineno, gluErrorString(error));
}
#define glStencilOp(fail, zfail, zpass) glStencilOpError(fail, zfail, zpass, __FILE__, __LINE__)

inline void glTexCoordPointerError(GLint  size, GLenum  type, GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glTexCoordPointer(size, type, stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoordPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoordPointer(size, type, stride, pointer) glTexCoordPointerError(size, type, stride, pointer, __FILE__, __LINE__)

inline void glTexEnvfError(GLenum  target, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glTexEnvf(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnvf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnvf(target, pname, param) glTexEnvfError(target, pname, param, __FILE__, __LINE__)

inline void glTexEnvfvError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glTexEnvfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnvfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnvfv(target, pname, params) glTexEnvfvError(target, pname, params, __FILE__, __LINE__)

inline void glTexEnviError(GLenum  target, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glTexEnvi(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnvi]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnvi(target, pname, param) glTexEnviError(target, pname, param, __FILE__, __LINE__)

inline void glTexEnvivError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glTexEnviv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnviv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnviv(target, pname, params) glTexEnvivError(target, pname, params, __FILE__, __LINE__)

inline void glTexGendError(GLenum  coord, GLenum  pname, GLdouble  param, const char *filename, int lineno)
{
  glTexGend(coord, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGend]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGend(coord, pname, param) glTexGendError(coord, pname, param, __FILE__, __LINE__)

inline void glTexGendvError(GLenum  coord, GLenum  pname, const  GLdouble * params, const char *filename, int lineno)
{
  glTexGendv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGendv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGendv(coord, pname, params) glTexGendvError(coord, pname, params, __FILE__, __LINE__)

inline void glTexGenfError(GLenum  coord, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glTexGenf(coord, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGenf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGenf(coord, pname, param) glTexGenfError(coord, pname, param, __FILE__, __LINE__)

inline void glTexGenfvError(GLenum  coord, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glTexGenfv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGenfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGenfv(coord, pname, params) glTexGenfvError(coord, pname, params, __FILE__, __LINE__)

inline void glTexGeniError(GLenum  coord, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glTexGeni(coord, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGeni]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGeni(coord, pname, param) glTexGeniError(coord, pname, param, __FILE__, __LINE__)

inline void glTexGenivError(GLenum  coord, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glTexGeniv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGeniv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGeniv(coord, pname, params) glTexGenivError(coord, pname, params, __FILE__, __LINE__)

inline void glTexImage1DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLint  border, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexImage1D(target, level, internalformat, width, border, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexImage1D(target, level, internalformat, width, border, format, type, pixels) glTexImage1DError(target, level, internalformat, width, border, format, type, pixels, __FILE__, __LINE__)

inline void glTexImage2DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLint  border, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels) glTexImage2DError(target, level, internalformat, width, height, border, format, type, pixels, __FILE__, __LINE__)

inline void glTexImage3DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLsizei  depth, GLint  border, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels) glTexImage3DError(target, level, internalformat, width, height, depth, border, format, type, pixels, __FILE__, __LINE__)

inline void glTexParameterfError(GLenum  target, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glTexParameterf(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameterf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameterf(target, pname, param) glTexParameterfError(target, pname, param, __FILE__, __LINE__)

inline void glTexParameterfvError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glTexParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameterfv(target, pname, params) glTexParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glTexParameteriError(GLenum  target, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glTexParameteri(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameteri]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameteri(target, pname, param) glTexParameteriError(target, pname, param, __FILE__, __LINE__)

inline void glTexParameterivError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glTexParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameteriv(target, pname, params) glTexParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glTexSubImage1DError(GLenum  target, GLint  level, GLint  xoffset, GLsizei  width, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexSubImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexSubImage1D(target, level, xoffset, width, format, type, pixels) glTexSubImage1DError(target, level, xoffset, width, format, type, pixels, __FILE__, __LINE__)

inline void glTexSubImage2DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexSubImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) glTexSubImage2DError(target, level, xoffset, yoffset, width, height, format, type, pixels, __FILE__, __LINE__)

inline void glTexSubImage3DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLsizei  width, GLsizei  height, GLsizei  depth, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexSubImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) glTexSubImage3DError(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels, __FILE__, __LINE__)

inline void glTranslatedError(GLdouble  x, GLdouble  y, GLdouble  z, const char *filename, int lineno)
{
  glTranslated(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTranslated]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTranslated(x, y, z) glTranslatedError(x, y, z, __FILE__, __LINE__)

inline void glTranslatefError(GLfloat  x, GLfloat  y, GLfloat  z, const char *filename, int lineno)
{
  glTranslatef(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTranslatef]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTranslatef(x, y, z) glTranslatefError(x, y, z, __FILE__, __LINE__)

inline void glVertexPointerError(GLint  size, GLenum  type, GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glVertexPointer(size, type, stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertexPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertexPointer(size, type, stride, pointer) glVertexPointerError(size, type, stride, pointer, __FILE__, __LINE__)

inline void glViewportError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glViewport(x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glViewport]: %s\n", filename, lineno, gluErrorString(error));
}
#define glViewport(x, y, width, height) glViewportError(x, y, width, height, __FILE__, __LINE__)

inline void glMultiTexCoord1dARBError(GLenum  texture, GLdouble  s, const char *filename, int lineno)
{
  glMultiTexCoord1dARB(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1dARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1dARB(texture, s) glMultiTexCoord1dARBError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1dvARBError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord1dvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1dvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1dvARB(texture, v) glMultiTexCoord1dvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord1fARBError(GLenum  texture, GLfloat  s, const char *filename, int lineno)
{
  glMultiTexCoord1fARB(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1fARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1fARB(texture, s) glMultiTexCoord1fARBError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1fvARBError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord1fvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1fvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1fvARB(texture, v) glMultiTexCoord1fvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord1iARBError(GLenum  texture, GLint  s, const char *filename, int lineno)
{
  glMultiTexCoord1iARB(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1iARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1iARB(texture, s) glMultiTexCoord1iARBError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1ivARBError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord1ivARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1ivARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1ivARB(texture, v) glMultiTexCoord1ivARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord1sARBError(GLenum  texture, GLshort  s, const char *filename, int lineno)
{
  glMultiTexCoord1sARB(texture, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1sARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1sARB(texture, s) glMultiTexCoord1sARBError(texture, s, __FILE__, __LINE__)

inline void glMultiTexCoord1svARBError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord1svARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1svARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1svARB(texture, v) glMultiTexCoord1svARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2dARBError(GLenum  texture, GLdouble  s, GLdouble  t, const char *filename, int lineno)
{
  glMultiTexCoord2dARB(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2dARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2dARB(texture, s, t) glMultiTexCoord2dARBError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2dvARBError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord2dvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2dvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2dvARB(texture, v) glMultiTexCoord2dvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2fARBError(GLenum  texture, GLfloat  s, GLfloat  t, const char *filename, int lineno)
{
  glMultiTexCoord2fARB(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2fARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2fARB(texture, s, t) glMultiTexCoord2fARBError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2fvARBError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord2fvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2fvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2fvARB(texture, v) glMultiTexCoord2fvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2iARBError(GLenum  texture, GLint  s, GLint  t, const char *filename, int lineno)
{
  glMultiTexCoord2iARB(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2iARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2iARB(texture, s, t) glMultiTexCoord2iARBError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2ivARBError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord2ivARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2ivARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2ivARB(texture, v) glMultiTexCoord2ivARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord2sARBError(GLenum  texture, GLshort  s, GLshort  t, const char *filename, int lineno)
{
  glMultiTexCoord2sARB(texture, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2sARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2sARB(texture, s, t) glMultiTexCoord2sARBError(texture, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2svARBError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord2svARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2svARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2svARB(texture, v) glMultiTexCoord2svARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3dARBError(GLenum  texture, GLdouble  s, GLdouble  t, GLdouble  r, const char *filename, int lineno)
{
  glMultiTexCoord3dARB(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3dARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3dARB(texture, s, t, r) glMultiTexCoord3dARBError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3dvARBError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord3dvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3dvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3dvARB(texture, v) glMultiTexCoord3dvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3fARBError(GLenum  texture, GLfloat  s, GLfloat  t, GLfloat  r, const char *filename, int lineno)
{
  glMultiTexCoord3fARB(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3fARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3fARB(texture, s, t, r) glMultiTexCoord3fARBError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3fvARBError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord3fvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3fvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3fvARB(texture, v) glMultiTexCoord3fvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3iARBError(GLenum  texture, GLint  s, GLint  t, GLint  r, const char *filename, int lineno)
{
  glMultiTexCoord3iARB(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3iARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3iARB(texture, s, t, r) glMultiTexCoord3iARBError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3ivARBError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord3ivARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3ivARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3ivARB(texture, v) glMultiTexCoord3ivARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord3sARBError(GLenum  texture, GLshort  s, GLshort  t, GLshort  r, const char *filename, int lineno)
{
  glMultiTexCoord3sARB(texture, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3sARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3sARB(texture, s, t, r) glMultiTexCoord3sARBError(texture, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3svARBError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord3svARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3svARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3svARB(texture, v) glMultiTexCoord3svARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4dARBError(GLenum  texture, GLdouble  s, GLdouble  t, GLdouble  r, GLdouble  q, const char *filename, int lineno)
{
  glMultiTexCoord4dARB(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4dARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4dARB(texture, s, t, r, q) glMultiTexCoord4dARBError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4dvARBError(GLenum  texture, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord4dvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4dvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4dvARB(texture, v) glMultiTexCoord4dvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4fARBError(GLenum  texture, GLfloat  s, GLfloat  t, GLfloat  r, GLfloat  q, const char *filename, int lineno)
{
  glMultiTexCoord4fARB(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4fARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4fARB(texture, s, t, r, q) glMultiTexCoord4fARBError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4fvARBError(GLenum  texture, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord4fvARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4fvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4fvARB(texture, v) glMultiTexCoord4fvARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4iARBError(GLenum  texture, GLint  s, GLint  t, GLint  r, GLint  q, const char *filename, int lineno)
{
  glMultiTexCoord4iARB(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4iARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4iARB(texture, s, t, r, q) glMultiTexCoord4iARBError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4ivARBError(GLenum  texture, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord4ivARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4ivARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4ivARB(texture, v) glMultiTexCoord4ivARBError(texture, v, __FILE__, __LINE__)

inline void glMultiTexCoord4sARBError(GLenum  texture, GLshort  s, GLshort  t, GLshort  r, GLshort  q, const char *filename, int lineno)
{
  glMultiTexCoord4sARB(texture, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4sARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4sARB(texture, s, t, r, q) glMultiTexCoord4sARBError(texture, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4svARBError(GLenum  texture, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord4svARB(texture, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4svARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4svARB(texture, v) glMultiTexCoord4svARBError(texture, v, __FILE__, __LINE__)

inline void glClientActiveTextureARBError(GLenum  texture, const char *filename, int lineno)
{
  glClientActiveTextureARB(texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClientActiveTextureARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClientActiveTextureARB(texture) glClientActiveTextureARBError(texture, __FILE__, __LINE__)

inline void glActiveTextureARBError(GLenum  texture, const char *filename, int lineno)
{
  glActiveTextureARB(texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glActiveTextureARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glActiveTextureARB(texture) glActiveTextureARBError(texture, __FILE__, __LINE__)

inline void glCompressedTexImage1DARBError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLint  border, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexImage1DARB(target, level, internalformat, width, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage1DARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage1DARB(target, level, internalformat, width, border, imageSize, data) glCompressedTexImage1DARBError(target, level, internalformat, width, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexImage2DARBError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLint  border, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexImage2DARB(target, level, internalformat, width, height, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage2DARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage2DARB(target, level, internalformat, width, height, border, imageSize, data) glCompressedTexImage2DARBError(target, level, internalformat, width, height, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexImage3DARBError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLsizei  depth, GLint  border, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexImage3DARB(target, level, internalformat, width, height, depth, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage3DARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage3DARB(target, level, internalformat, width, height, depth, border, imageSize, data) glCompressedTexImage3DARBError(target, level, internalformat, width, height, depth, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage1DARBError(GLenum  target, GLint  level, GLint  xoffset, GLsizei  width, GLenum  format, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexSubImage1DARB(target, level, xoffset, width, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage1DARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage1DARB(target, level, xoffset, width, format, imageSize, data) glCompressedTexSubImage1DARBError(target, level, xoffset, width, format, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage2DARBError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLsizei  width, GLsizei  height, GLenum  format, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexSubImage2DARB(target, level, xoffset, yoffset, width, height, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage2DARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage2DARB(target, level, xoffset, yoffset, width, height, format, imageSize, data) glCompressedTexSubImage2DARBError(target, level, xoffset, yoffset, width, height, format, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage3DARBError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLsizei  width, GLsizei  height, GLsizei  depth, GLenum  format, GLsizei  imageSize, const  void * data, const char *filename, int lineno)
{
  glCompressedTexSubImage3DARB(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage3DARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage3DARB(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) glCompressedTexSubImage3DARBError(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data, __FILE__, __LINE__)

inline void glGetCompressedTexImageARBError(GLenum  target, GLint  lod, GLvoid * img, const char *filename, int lineno)
{
  glGetCompressedTexImageARB(target, lod, img);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetCompressedTexImageARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetCompressedTexImageARB(target, lod, img) glGetCompressedTexImageARBError(target, lod, img, __FILE__, __LINE__)

inline void glLoadTransposeMatrixdARBError(const  GLdouble * m, const char *filename, int lineno)
{
  glLoadTransposeMatrixdARB(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadTransposeMatrixdARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadTransposeMatrixdARB(m) glLoadTransposeMatrixdARBError(m, __FILE__, __LINE__)

inline void glLoadTransposeMatrixfARBError(const  GLfloat * m, const char *filename, int lineno)
{
  glLoadTransposeMatrixfARB(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadTransposeMatrixfARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadTransposeMatrixfARB(m) glLoadTransposeMatrixfARBError(m, __FILE__, __LINE__)

inline void glMultTransposeMatrixdARBError(const  GLdouble * m, const char *filename, int lineno)
{
  glMultTransposeMatrixdARB(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultTransposeMatrixdARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultTransposeMatrixdARB(m) glMultTransposeMatrixdARBError(m, __FILE__, __LINE__)

inline void glMultTransposeMatrixfARBError(const  GLfloat * m, const char *filename, int lineno)
{
  glMultTransposeMatrixfARB(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultTransposeMatrixfARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultTransposeMatrixfARB(m) glMultTransposeMatrixfARBError(m, __FILE__, __LINE__)

inline void glSampleCoverageARBError(GLclampf  value, GLboolean  invert, const char *filename, int lineno)
{
  glSampleCoverageARB(value, invert);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSampleCoverageARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSampleCoverageARB(value, invert) glSampleCoverageARBError(value, invert, __FILE__, __LINE__)

inline void glPointParameterfARBError(GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glPointParameterfARB(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPointParameterfARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPointParameterfARB(pname, param) glPointParameterfARBError(pname, param, __FILE__, __LINE__)

inline void glPointParameterfvARBError(GLenum  pname, const  GLfloat * param, const char *filename, int lineno)
{
  glPointParameterfvARB(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPointParameterfvARB]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPointParameterfvARB(pname, param) glPointParameterfvARBError(pname, param, __FILE__, __LINE__)

inline void glPolygonOffsetEXTError(GLfloat  factor, GLfloat  bias, const char *filename, int lineno)
{
  glPolygonOffsetEXT(factor, bias);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPolygonOffsetEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPolygonOffsetEXT(factor, bias) glPolygonOffsetEXTError(factor, bias, __FILE__, __LINE__)

inline void glTexImage3DEXTError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLsizei  depth, GLint  border, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexImage3DEXT(target, level, internalformat, width, height, depth, border, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexImage3DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexImage3DEXT(target, level, internalformat, width, height, depth, border, format, type, pixels) glTexImage3DEXTError(target, level, internalformat, width, height, depth, border, format, type, pixels, __FILE__, __LINE__)

inline void glTexSubImage3DEXTError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLsizei  width, GLsizei  height, GLsizei  depth, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexSubImage3DEXT(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexSubImage3DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexSubImage3DEXT(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) glTexSubImage3DEXTError(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels, __FILE__, __LINE__)

inline void glCopyTexSubImage3DEXTError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glCopyTexSubImage3DEXT(target, level, xoffset, yoffset, zoffset, x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexSubImage3DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexSubImage3DEXT(target, level, xoffset, yoffset, zoffset, x, y, width, height) glCopyTexSubImage3DEXTError(target, level, xoffset, yoffset, zoffset, x, y, width, height, __FILE__, __LINE__)

inline void glHistogramEXTError(GLenum  target, GLsizei  width, GLenum  internalformat, GLboolean  sink, const char *filename, int lineno)
{
  glHistogramEXT(target, width, internalformat, sink);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glHistogramEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glHistogramEXT(target, width, internalformat, sink) glHistogramEXTError(target, width, internalformat, sink, __FILE__, __LINE__)

inline void glResetHistogramEXTError(GLenum  target, const char *filename, int lineno)
{
  glResetHistogramEXT(target);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glResetHistogramEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glResetHistogramEXT(target) glResetHistogramEXTError(target, __FILE__, __LINE__)

inline void glGetHistogramEXTError(GLenum  target, GLboolean  reset, GLenum  format, GLenum  type, GLvoid * values, const char *filename, int lineno)
{
  glGetHistogramEXT(target, reset, format, type, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetHistogramEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetHistogramEXT(target, reset, format, type, values) glGetHistogramEXTError(target, reset, format, type, values, __FILE__, __LINE__)

inline void glGetHistogramParameterfvEXTError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetHistogramParameterfvEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetHistogramParameterfvEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetHistogramParameterfvEXT(target, pname, params) glGetHistogramParameterfvEXTError(target, pname, params, __FILE__, __LINE__)

inline void glGetHistogramParameterivEXTError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetHistogramParameterivEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetHistogramParameterivEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetHistogramParameterivEXT(target, pname, params) glGetHistogramParameterivEXTError(target, pname, params, __FILE__, __LINE__)

inline void glMinmaxEXTError(GLenum  target, GLenum  internalformat, GLboolean  sink, const char *filename, int lineno)
{
  glMinmaxEXT(target, internalformat, sink);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMinmaxEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMinmaxEXT(target, internalformat, sink) glMinmaxEXTError(target, internalformat, sink, __FILE__, __LINE__)

inline void glResetMinmaxEXTError(GLenum  target, const char *filename, int lineno)
{
  glResetMinmaxEXT(target);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glResetMinmaxEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glResetMinmaxEXT(target) glResetMinmaxEXTError(target, __FILE__, __LINE__)

inline void glGetMinmaxEXTError(GLenum  target, GLboolean  reset, GLenum  format, GLenum  type, GLvoid * values, const char *filename, int lineno)
{
  glGetMinmaxEXT(target, reset, format, type, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMinmaxEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMinmaxEXT(target, reset, format, type, values) glGetMinmaxEXTError(target, reset, format, type, values, __FILE__, __LINE__)

inline void glGetMinmaxParameterfvEXTError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetMinmaxParameterfvEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMinmaxParameterfvEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMinmaxParameterfvEXT(target, pname, params) glGetMinmaxParameterfvEXTError(target, pname, params, __FILE__, __LINE__)

inline void glGetMinmaxParameterivEXTError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetMinmaxParameterivEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMinmaxParameterivEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMinmaxParameterivEXT(target, pname, params) glGetMinmaxParameterivEXTError(target, pname, params, __FILE__, __LINE__)

inline void glBlendFuncSeparateEXTError(GLenum  sfactorRGB, GLenum  dfactorRGB, GLenum  sfactorAlpha, GLenum  dfactorAlpha, const char *filename, int lineno)
{
  glBlendFuncSeparateEXT(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendFuncSeparateEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendFuncSeparateEXT(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) glBlendFuncSeparateEXTError(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha, __FILE__, __LINE__)

inline void glBlendColorEXTError(GLclampf  red, GLclampf  green, GLclampf  blue, GLclampf  alpha, const char *filename, int lineno)
{
  glBlendColorEXT(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendColorEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendColorEXT(red, green, blue, alpha) glBlendColorEXTError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glBlendEquationEXTError(GLenum  mode, const char *filename, int lineno)
{
  glBlendEquationEXT(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendEquationEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendEquationEXT(mode) glBlendEquationEXTError(mode, __FILE__, __LINE__)

inline void glConvolutionParameterfEXTError(GLenum  target, GLenum  pname, GLfloat  params, const char *filename, int lineno)
{
  glConvolutionParameterfEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameterfEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameterfEXT(target, pname, params) glConvolutionParameterfEXTError(target, pname, params, __FILE__, __LINE__)

inline void glConvolutionParameterfvEXTError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glConvolutionParameterfvEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameterfvEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameterfvEXT(target, pname, params) glConvolutionParameterfvEXTError(target, pname, params, __FILE__, __LINE__)

inline void glConvolutionParameteriEXTError(GLenum  target, GLenum  pname, GLint  params, const char *filename, int lineno)
{
  glConvolutionParameteriEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameteriEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameteriEXT(target, pname, params) glConvolutionParameteriEXTError(target, pname, params, __FILE__, __LINE__)

inline void glConvolutionParameterivEXTError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glConvolutionParameterivEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionParameterivEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionParameterivEXT(target, pname, params) glConvolutionParameterivEXTError(target, pname, params, __FILE__, __LINE__)

inline void glConvolutionFilter1DEXTError(GLenum  target, GLenum  internalformat, GLsizei  width, GLenum  format, GLenum  type, const  GLvoid * image, const char *filename, int lineno)
{
  glConvolutionFilter1DEXT(target, internalformat, width, format, type, image);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionFilter1DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionFilter1DEXT(target, internalformat, width, format, type, image) glConvolutionFilter1DEXTError(target, internalformat, width, format, type, image, __FILE__, __LINE__)

inline void glConvolutionFilter2DEXTError(GLenum  target, GLenum  internalformat, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, const  GLvoid * image, const char *filename, int lineno)
{
  glConvolutionFilter2DEXT(target, internalformat, width, height, format, type, image);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glConvolutionFilter2DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glConvolutionFilter2DEXT(target, internalformat, width, height, format, type, image) glConvolutionFilter2DEXTError(target, internalformat, width, height, format, type, image, __FILE__, __LINE__)

inline void glCopyConvolutionFilter1DEXTError(GLenum  target, GLenum  internalformat, GLint  x, GLint  y, GLsizei  width, const char *filename, int lineno)
{
  glCopyConvolutionFilter1DEXT(target, internalformat, x, y, width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyConvolutionFilter1DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyConvolutionFilter1DEXT(target, internalformat, x, y, width) glCopyConvolutionFilter1DEXTError(target, internalformat, x, y, width, __FILE__, __LINE__)

inline void glCopyConvolutionFilter2DEXTError(GLenum  target, GLenum  internalformat, GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glCopyConvolutionFilter2DEXT(target, internalformat, x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyConvolutionFilter2DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyConvolutionFilter2DEXT(target, internalformat, x, y, width, height) glCopyConvolutionFilter2DEXTError(target, internalformat, x, y, width, height, __FILE__, __LINE__)

inline void glSeparableFilter2DEXTError(GLenum  target, GLenum  internalformat, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, GLvoid * row, GLvoid * column, const char *filename, int lineno)
{
  glSeparableFilter2DEXT(target, internalformat, width, height, format, type, row, column);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSeparableFilter2DEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSeparableFilter2DEXT(target, internalformat, width, height, format, type, row, column) glSeparableFilter2DEXTError(target, internalformat, width, height, format, type, row, column, __FILE__, __LINE__)

inline void glGetConvolutionParameterfvEXTError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetConvolutionParameterfvEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetConvolutionParameterfvEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetConvolutionParameterfvEXT(target, pname, params) glGetConvolutionParameterfvEXTError(target, pname, params, __FILE__, __LINE__)

inline void glGetConvolutionParameterivEXTError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetConvolutionParameterivEXT(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetConvolutionParameterivEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetConvolutionParameterivEXT(target, pname, params) glGetConvolutionParameterivEXTError(target, pname, params, __FILE__, __LINE__)

inline void glGetConvolutionFilterEXTError(GLenum  target, GLenum  format, GLenum  type, GLvoid * image, const char *filename, int lineno)
{
  glGetConvolutionFilterEXT(target, format, type, image);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetConvolutionFilterEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetConvolutionFilterEXT(target, format, type, image) glGetConvolutionFilterEXTError(target, format, type, image, __FILE__, __LINE__)

inline void glGetSeparableFilterEXTError(GLenum  target, GLenum  format, GLenum  type, GLvoid * row, GLvoid * column, GLvoid * span, const char *filename, int lineno)
{
  glGetSeparableFilterEXT(target, format, type, row, column, span);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetSeparableFilterEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetSeparableFilterEXT(target, format, type, row, column, span) glGetSeparableFilterEXTError(target, format, type, row, column, span, __FILE__, __LINE__)

inline void glPixelTransformParameteriEXTError(GLenum  target, GLenum  pname, const  int  param, const char *filename, int lineno)
{
  glPixelTransformParameteriEXT(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelTransformParameteriEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelTransformParameteriEXT(target, pname, param) glPixelTransformParameteriEXTError(target, pname, param, __FILE__, __LINE__)

inline void glPixelTransformParameterfEXTError(GLenum  target, GLenum  pname, const  float  param, const char *filename, int lineno)
{
  glPixelTransformParameterfEXT(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelTransformParameterfEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelTransformParameterfEXT(target, pname, param) glPixelTransformParameterfEXTError(target, pname, param, __FILE__, __LINE__)

inline void glPixelTransformParameterivEXTError(GLenum  target, GLenum  pname, const  int * param, const char *filename, int lineno)
{
  glPixelTransformParameterivEXT(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelTransformParameterivEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelTransformParameterivEXT(target, pname, param) glPixelTransformParameterivEXTError(target, pname, param, __FILE__, __LINE__)

inline void glPixelTransformParameterfvEXTError(GLenum  target, GLenum  pname, const  float * param, const char *filename, int lineno)
{
  glPixelTransformParameterfvEXT(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelTransformParameterfvEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelTransformParameterfvEXT(target, pname, param) glPixelTransformParameterfvEXTError(target, pname, param, __FILE__, __LINE__)

inline void glGetPixelTransformParameterivEXTError(GLenum  target, GLenum  pname, GLint * param, const char *filename, int lineno)
{
  glGetPixelTransformParameterivEXT(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPixelTransformParameterivEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPixelTransformParameterivEXT(target, pname, param) glGetPixelTransformParameterivEXTError(target, pname, param, __FILE__, __LINE__)

inline void glGetPixelTransformParameterfvEXTError(GLenum  target, GLenum  pname, GLfloat * param, const char *filename, int lineno)
{
  glGetPixelTransformParameterfvEXT(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPixelTransformParameterfvEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPixelTransformParameterfvEXT(target, pname, param) glGetPixelTransformParameterfvEXTError(target, pname, param, __FILE__, __LINE__)

inline void glLockArraysEXTError(GLint  first, GLsizei  count, const char *filename, int lineno)
{
  glLockArraysEXT(first, count);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLockArraysEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLockArraysEXT(first, count) glLockArraysEXTError(first, count, __FILE__, __LINE__)

inline void glUnlockArraysEXTError(const char *filename, int lineno)
{
  glUnlockArraysEXT();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glUnlockArraysEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glUnlockArraysEXT() glUnlockArraysEXTError(__FILE__, __LINE__)

inline void glMultiDrawArraysEXTError(GLenum  mode, GLint*  first, GLsizei*  count, GLsizei  primcount, const char *filename, int lineno)
{
  glMultiDrawArraysEXT(mode, first, count, primcount);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiDrawArraysEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiDrawArraysEXT(mode, first, count, primcount) glMultiDrawArraysEXTError(mode, first, count, primcount, __FILE__, __LINE__)

inline void glMultiDrawElementsEXTError(GLenum  mode, GLsizei * count, GLenum  type, const  GLvoid ** indices, GLsizei  primcount, const char *filename, int lineno)
{
  glMultiDrawElementsEXT(mode, count, type, indices, primcount);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiDrawElementsEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiDrawElementsEXT(mode, count, type, indices, primcount) glMultiDrawElementsEXTError(mode, count, type, indices, primcount, __FILE__, __LINE__)

inline void glClearColorModeEXTError(GLenum  pname, const char *filename, int lineno)
{
  glClearColorModeEXT(pname);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearColorModeEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearColorModeEXT(pname) glClearColorModeEXTError(pname, __FILE__, __LINE__)

inline void glClearParameterfvEXTError(GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glClearParameterfvEXT(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearParameterfvEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearParameterfvEXT(pname, params) glClearParameterfvEXTError(pname, params, __FILE__, __LINE__)

inline void glClearParameterivEXTError(GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glClearParameterivEXT(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearParameterivEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearParameterivEXT(pname, params) glClearParameterivEXTError(pname, params, __FILE__, __LINE__)

inline void glClearParameterfEXTError(GLenum  pname, const  GLfloat  param, const char *filename, int lineno)
{
  glClearParameterfEXT(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearParameterfEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearParameterfEXT(pname, param) glClearParameterfEXTError(pname, param, __FILE__, __LINE__)

inline void glClearParameteriEXTError(GLenum  pname, const  GLint  param, const char *filename, int lineno)
{
  glClearParameteriEXT(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearParameteriEXT]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearParameteriEXT(pname, param) glClearParameteriEXTError(pname, param, __FILE__, __LINE__)

inline void glColorTableSGIError(GLenum  target, GLenum  internalFormat, GLsizei  width, GLenum  format, GLenum  type, const  GLvoid*  table, const char *filename, int lineno)
{
  glColorTableSGI(target, internalFormat, width, format, type, table);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTableSGI]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTableSGI(target, internalFormat, width, format, type, table) glColorTableSGIError(target, internalFormat, width, format, type, table, __FILE__, __LINE__)

inline void glCopyColorTableSGIError(GLenum  target, GLenum  internalFormat, GLint  x, GLint  y, GLsizei  width, const char *filename, int lineno)
{
  glCopyColorTableSGI(target, internalFormat, x, y, width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyColorTableSGI]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyColorTableSGI(target, internalFormat, x, y, width) glCopyColorTableSGIError(target, internalFormat, x, y, width, __FILE__, __LINE__)

inline void glColorTableParameterfvSGIError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glColorTableParameterfvSGI(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTableParameterfvSGI]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTableParameterfvSGI(target, pname, params) glColorTableParameterfvSGIError(target, pname, params, __FILE__, __LINE__)

inline void glColorTableParameterivSGIError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glColorTableParameterivSGI(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTableParameterivSGI]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTableParameterivSGI(target, pname, params) glColorTableParameterivSGIError(target, pname, params, __FILE__, __LINE__)

inline void glGetColorTableSGIError(GLenum  target, GLenum  format, GLenum  type, GLvoid * table, const char *filename, int lineno)
{
  glGetColorTableSGI(target, format, type, table);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetColorTableSGI]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetColorTableSGI(target, format, type, table) glGetColorTableSGIError(target, format, type, table, __FILE__, __LINE__)

inline void glGetColorTableParameterfvSGIError(GLenum  target, GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetColorTableParameterfvSGI(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetColorTableParameterfvSGI]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetColorTableParameterfvSGI(target, pname, params) glGetColorTableParameterfvSGIError(target, pname, params, __FILE__, __LINE__)

inline void glGetColorTableParameterivSGIError(GLenum  target, GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetColorTableParameterivSGI(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetColorTableParameterivSGI]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetColorTableParameterivSGI(target, pname, params) glGetColorTableParameterivSGIError(target, pname, params, __FILE__, __LINE__)

inline void glSharpenTexFuncSGISError(GLenum  target, GLsizei  n, const  GLfloat*  points, const char *filename, int lineno)
{
  glSharpenTexFuncSGIS(target, n, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSharpenTexFuncSGIS]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSharpenTexFuncSGIS(target, n, points) glSharpenTexFuncSGISError(target, n, points, __FILE__, __LINE__)

inline void glGetSharpenTexFuncSGISError(GLenum  target, GLfloat*  points, const char *filename, int lineno)
{
  glGetSharpenTexFuncSGIS(target, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetSharpenTexFuncSGIS]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetSharpenTexFuncSGIS(target, points) glGetSharpenTexFuncSGISError(target, points, __FILE__, __LINE__)

inline void glDetailTexFuncSGISError(GLenum  target, GLsizei  n, const  GLfloat*  points, const char *filename, int lineno)
{
  glDetailTexFuncSGIS(target, n, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDetailTexFuncSGIS]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDetailTexFuncSGIS(target, n, points) glDetailTexFuncSGISError(target, n, points, __FILE__, __LINE__)

inline void glGetDetailTexFuncSGISError(GLenum  target, GLfloat*  points, const char *filename, int lineno)
{
  glGetDetailTexFuncSGIS(target, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetDetailTexFuncSGIS]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetDetailTexFuncSGIS(target, points) glGetDetailTexFuncSGISError(target, points, __FILE__, __LINE__)

inline void glTexFilterFuncSGISError(GLenum  target, GLenum  filter, GLsizei  n, const  GLfloat*  points, const char *filename, int lineno)
{
  glTexFilterFuncSGIS(target, filter, n, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexFilterFuncSGIS]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexFilterFuncSGIS(target, filter, n, points) glTexFilterFuncSGISError(target, filter, n, points, __FILE__, __LINE__)

inline void glGetTexFilterFuncSGISError(GLenum  target, GLenum  filter, GLfloat*  points, const char *filename, int lineno)
{
  glGetTexFilterFuncSGIS(target, filter, points);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexFilterFuncSGIS]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexFilterFuncSGIS(target, filter, points) glGetTexFilterFuncSGISError(target, filter, points, __FILE__, __LINE__)

inline void glMultiDrawArraysSUNError(GLenum  mode, GLint*  first, GLsizei*  count, GLsizei  primcount, const char *filename, int lineno)
{
  glMultiDrawArraysSUN(mode, first, count, primcount);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiDrawArraysSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiDrawArraysSUN(mode, first, count, primcount) glMultiDrawArraysSUNError(mode, first, count, primcount, __FILE__, __LINE__)

inline void glMultiDrawElementsSUNError(GLenum  mode, GLsizei * count, GLenum  type, const  GLvoid ** indices, GLsizei  primcount, const char *filename, int lineno)
{
  glMultiDrawElementsSUN(mode, count, type, indices, primcount);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiDrawElementsSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiDrawElementsSUN(mode, count, type, indices, primcount) glMultiDrawElementsSUNError(mode, count, type, indices, primcount, __FILE__, __LINE__)

inline void glDrawCompressedGeomSUNXError(GLint  size, GLubyte * data, const char *filename, int lineno)
{
  glDrawCompressedGeomSUNX(size, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawCompressedGeomSUNX]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawCompressedGeomSUNX(size, data) glDrawCompressedGeomSUNXError(size, data, __FILE__, __LINE__)

inline void glReplacementCodePointerSUNError(GLenum arg1, GLsizei arg2, const  void * arg3, const char *filename, int lineno)
{
  glReplacementCodePointerSUN(arg1, arg2, arg3);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glReplacementCodePointerSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glReplacementCodePointerSUN(arg1, arg2, arg3) glReplacementCodePointerSUNError(arg1, arg2, arg3, __FILE__, __LINE__)

inline void glDrawMeshArraysSUNError(GLenum arg1, GLint arg2, GLsizei arg3, GLsizei arg4, const char *filename, int lineno)
{
  glDrawMeshArraysSUN(arg1, arg2, arg3, arg4);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawMeshArraysSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawMeshArraysSUN(arg1, arg2, arg3, arg4) glDrawMeshArraysSUNError(arg1, arg2, arg3, arg4, __FILE__, __LINE__)

inline void glGlobalAlphaFactorbSUNError(GLbyte  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactorbSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactorbSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactorbSUN(factor) glGlobalAlphaFactorbSUNError(factor, __FILE__, __LINE__)

inline void glGlobalAlphaFactorsSUNError(GLshort  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactorsSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactorsSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactorsSUN(factor) glGlobalAlphaFactorsSUNError(factor, __FILE__, __LINE__)

inline void glGlobalAlphaFactoriSUNError(GLint  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactoriSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactoriSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactoriSUN(factor) glGlobalAlphaFactoriSUNError(factor, __FILE__, __LINE__)

inline void glGlobalAlphaFactorfSUNError(GLfloat  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactorfSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactorfSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactorfSUN(factor) glGlobalAlphaFactorfSUNError(factor, __FILE__, __LINE__)

inline void glGlobalAlphaFactordSUNError(GLdouble  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactordSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactordSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactordSUN(factor) glGlobalAlphaFactordSUNError(factor, __FILE__, __LINE__)

inline void glGlobalAlphaFactorubSUNError(GLubyte  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactorubSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactorubSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactorubSUN(factor) glGlobalAlphaFactorubSUNError(factor, __FILE__, __LINE__)

inline void glGlobalAlphaFactorusSUNError(GLushort  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactorusSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactorusSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactorusSUN(factor) glGlobalAlphaFactorusSUNError(factor, __FILE__, __LINE__)

inline void glGlobalAlphaFactoruiSUNError(GLuint  factor, const char *filename, int lineno)
{
  glGlobalAlphaFactoruiSUN(factor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGlobalAlphaFactoruiSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGlobalAlphaFactoruiSUN(factor) glGlobalAlphaFactoruiSUNError(factor, __FILE__, __LINE__)

inline void glFinishTextureSUNXError(const char *filename, int lineno)
{
  glFinishTextureSUNX();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFinishTextureSUNX]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFinishTextureSUNX() glFinishTextureSUNXError(__FILE__, __LINE__)

inline void glReadVideoPixelsSUNError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, GLvoid * pixels, const char *filename, int lineno)
{
  glReadVideoPixelsSUN(x, y, width, height, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glReadVideoPixelsSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glReadVideoPixelsSUN(x, y, width, height, format, type, pixels) glReadVideoPixelsSUNError(x, y, width, height, format, type, pixels, __FILE__, __LINE__)

inline void glReadSamplesSUNError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, GLvoid * pixels, const char *filename, int lineno)
{
  glReadSamplesSUN(x, y, width, height, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glReadSamplesSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glReadSamplesSUN(x, y, width, height, format, type, pixels) glReadSamplesSUNError(x, y, width, height, format, type, pixels, __FILE__, __LINE__)

inline void glWriteSamplesSUNError(GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, const  GLvoid * samples, const char *filename, int lineno)
{
  glWriteSamplesSUN(width, height, format, type, samples);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glWriteSamplesSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glWriteSamplesSUN(width, height, format, type, samples) glWriteSamplesSUNError(width, height, format, type, samples, __FILE__, __LINE__)

inline void glSetTextureTargetiSUNError(GLenum  mode, GLuint  param, const char *filename, int lineno)
{
  glSetTextureTargetiSUN(mode, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSetTextureTargetiSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSetTextureTargetiSUN(mode, param) glSetTextureTargetiSUNError(mode, param, __FILE__, __LINE__)

inline void glSetTextureTargetModeSUNError(GLenum  mode, const char *filename, int lineno)
{
  glSetTextureTargetModeSUN(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSetTextureTargetModeSUN]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSetTextureTargetModeSUN(mode) glSetTextureTargetModeSUNError(mode, __FILE__, __LINE__)

#endif

#endif

#endif

