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

inline void glClearIndexError(GLfloat  c, const char *filename, int lineno)
{
  glClearIndex(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearIndex]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearIndex(c) glClearIndexError(c, __FILE__, __LINE__)

inline void glClearColorError(GLclampf  red, GLclampf  green, GLclampf  blue, GLclampf  alpha, const char *filename, int lineno)
{
  glClearColor(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearColor]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearColor(red, green, blue, alpha) glClearColorError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glClearError(GLbitfield  mask, const char *filename, int lineno)
{
  glClear(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClear]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClear(mask) glClearError(mask, __FILE__, __LINE__)

inline void glIndexMaskError(GLuint  mask, const char *filename, int lineno)
{
  glIndexMask(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexMask]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexMask(mask) glIndexMaskError(mask, __FILE__, __LINE__)

inline void glColorMaskError(GLboolean  red, GLboolean  green, GLboolean  blue, GLboolean  alpha, const char *filename, int lineno)
{
  glColorMask(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorMask]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorMask(red, green, blue, alpha) glColorMaskError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glAlphaFuncError(GLenum  func, GLclampf  ref, const char *filename, int lineno)
{
  glAlphaFunc(func, ref);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glAlphaFunc]: %s\n", filename, lineno, gluErrorString(error));
}
#define glAlphaFunc(func, ref) glAlphaFuncError(func, ref, __FILE__, __LINE__)

inline void glBlendFuncError(GLenum  sfactor, GLenum  dfactor, const char *filename, int lineno)
{
  glBlendFunc(sfactor, dfactor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendFunc]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendFunc(sfactor, dfactor) glBlendFuncError(sfactor, dfactor, __FILE__, __LINE__)

inline void glLogicOpError(GLenum  opcode, const char *filename, int lineno)
{
  glLogicOp(opcode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLogicOp]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLogicOp(opcode) glLogicOpError(opcode, __FILE__, __LINE__)

inline void glCullFaceError(GLenum  mode, const char *filename, int lineno)
{
  glCullFace(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCullFace]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCullFace(mode) glCullFaceError(mode, __FILE__, __LINE__)

inline void glFrontFaceError(GLenum  mode, const char *filename, int lineno)
{
  glFrontFace(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFrontFace]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFrontFace(mode) glFrontFaceError(mode, __FILE__, __LINE__)

inline void glPointSizeError(GLfloat  size, const char *filename, int lineno)
{
  glPointSize(size);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPointSize]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPointSize(size) glPointSizeError(size, __FILE__, __LINE__)

inline void glLineWidthError(GLfloat  width, const char *filename, int lineno)
{
  glLineWidth(width);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLineWidth]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLineWidth(width) glLineWidthError(width, __FILE__, __LINE__)

inline void glLineStippleError(GLint  factor, GLushort  pattern, const char *filename, int lineno)
{
  glLineStipple(factor, pattern);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLineStipple]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLineStipple(factor, pattern) glLineStippleError(factor, pattern, __FILE__, __LINE__)

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

inline void glGetPolygonStippleError(GLubyte * mask, const char *filename, int lineno)
{
  glGetPolygonStipple(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPolygonStipple]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPolygonStipple(mask) glGetPolygonStippleError(mask, __FILE__, __LINE__)

inline void glEdgeFlagError(GLboolean  flag, const char *filename, int lineno)
{
  glEdgeFlag(flag);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEdgeFlag]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEdgeFlag(flag) glEdgeFlagError(flag, __FILE__, __LINE__)

inline void glEdgeFlagvError(const  GLboolean * flag, const char *filename, int lineno)
{
  glEdgeFlagv(flag);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEdgeFlagv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEdgeFlagv(flag) glEdgeFlagvError(flag, __FILE__, __LINE__)

inline void glScissorError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glScissor(x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glScissor]: %s\n", filename, lineno, gluErrorString(error));
}
#define glScissor(x, y, width, height) glScissorError(x, y, width, height, __FILE__, __LINE__)

inline void glClipPlaneError(GLenum  plane, const  GLdouble * equation, const char *filename, int lineno)
{
  glClipPlane(plane, equation);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClipPlane]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClipPlane(plane, equation) glClipPlaneError(plane, equation, __FILE__, __LINE__)

inline void glGetClipPlaneError(GLenum  plane, GLdouble * equation, const char *filename, int lineno)
{
  glGetClipPlane(plane, equation);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetClipPlane]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetClipPlane(plane, equation) glGetClipPlaneError(plane, equation, __FILE__, __LINE__)

inline void glDrawBufferError(GLenum  mode, const char *filename, int lineno)
{
  glDrawBuffer(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawBuffer(mode) glDrawBufferError(mode, __FILE__, __LINE__)

inline void glReadBufferError(GLenum  mode, const char *filename, int lineno)
{
  glReadBuffer(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glReadBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glReadBuffer(mode) glReadBufferError(mode, __FILE__, __LINE__)

inline void glEnableError(GLenum  cap, const char *filename, int lineno)
{
  glEnable(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEnable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEnable(cap) glEnableError(cap, __FILE__, __LINE__)

inline void glDisableError(GLenum  cap, const char *filename, int lineno)
{
  glDisable(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDisable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDisable(cap) glDisableError(cap, __FILE__, __LINE__)

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

inline void glEnableClientStateError(GLenum  cap, const char *filename, int lineno)
{
  glEnableClientState(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEnableClientState]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEnableClientState(cap) glEnableClientStateError(cap, __FILE__, __LINE__)

inline void glDisableClientStateError(GLenum  cap, const char *filename, int lineno)
{
  glDisableClientState(cap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDisableClientState]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDisableClientState(cap) glDisableClientStateError(cap, __FILE__, __LINE__)

inline void glGetBooleanvError(GLenum  pname, GLboolean * params, const char *filename, int lineno)
{
  glGetBooleanv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetBooleanv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetBooleanv(pname, params) glGetBooleanvError(pname, params, __FILE__, __LINE__)

inline void glGetDoublevError(GLenum  pname, GLdouble * params, const char *filename, int lineno)
{
  glGetDoublev(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetDoublev]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetDoublev(pname, params) glGetDoublevError(pname, params, __FILE__, __LINE__)

inline void glGetFloatvError(GLenum  pname, GLfloat * params, const char *filename, int lineno)
{
  glGetFloatv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetFloatv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetFloatv(pname, params) glGetFloatvError(pname, params, __FILE__, __LINE__)

inline void glGetIntegervError(GLenum  pname, GLint * params, const char *filename, int lineno)
{
  glGetIntegerv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetIntegerv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetIntegerv(pname, params) glGetIntegervError(pname, params, __FILE__, __LINE__)

inline void glPushAttribError(GLbitfield  mask, const char *filename, int lineno)
{
  glPushAttrib(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushAttrib(mask) glPushAttribError(mask, __FILE__, __LINE__)

inline void glPopAttribError(const char *filename, int lineno)
{
  glPopAttrib();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopAttrib() glPopAttribError(__FILE__, __LINE__)

inline void glPushClientAttribError(GLbitfield  mask, const char *filename, int lineno)
{
  glPushClientAttrib(mask);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushClientAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushClientAttrib(mask) glPushClientAttribError(mask, __FILE__, __LINE__)

inline void glPopClientAttribError(const char *filename, int lineno)
{
  glPopClientAttrib();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopClientAttrib]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopClientAttrib() glPopClientAttribError(__FILE__, __LINE__)

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

inline const GLubyte* glGetStringError(GLenum  name, const char *filename, int lineno)
{
  const GLubyte* rv;
  rv = glGetString(name);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetString]: %s\n", filename, lineno, gluErrorString(error));
  return rv;
}
#define glGetString(name) glGetStringError(name, __FILE__, __LINE__)

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

inline void glHintError(GLenum  target, GLenum  mode, const char *filename, int lineno)
{
  glHint(target, mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glHint]: %s\n", filename, lineno, gluErrorString(error));
}
#define glHint(target, mode) glHintError(target, mode, __FILE__, __LINE__)

inline void glClearDepthError(GLclampd  depth, const char *filename, int lineno)
{
  glClearDepth(depth);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearDepth]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearDepth(depth) glClearDepthError(depth, __FILE__, __LINE__)

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

inline void glDepthRangeError(GLclampd  near_val, GLclampd  far_val, const char *filename, int lineno)
{
  glDepthRange(near_val, far_val);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDepthRange]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDepthRange(near_val, far_val) glDepthRangeError(near_val, far_val, __FILE__, __LINE__)

inline void glClearAccumError(GLfloat  red, GLfloat  green, GLfloat  blue, GLfloat  alpha, const char *filename, int lineno)
{
  glClearAccum(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearAccum]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearAccum(red, green, blue, alpha) glClearAccumError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glAccumError(GLenum  op, GLfloat  value, const char *filename, int lineno)
{
  glAccum(op, value);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glAccum]: %s\n", filename, lineno, gluErrorString(error));
}
#define glAccum(op, value) glAccumError(op, value, __FILE__, __LINE__)

inline void glMatrixModeError(GLenum  mode, const char *filename, int lineno)
{
  glMatrixMode(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMatrixMode]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMatrixMode(mode) glMatrixModeError(mode, __FILE__, __LINE__)

inline void glOrthoError(GLdouble  left, GLdouble  right, GLdouble  bottom, GLdouble  top, GLdouble  near_val, GLdouble  far_val, const char *filename, int lineno)
{
  glOrtho(left, right, bottom, top, near_val, far_val);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glOrtho]: %s\n", filename, lineno, gluErrorString(error));
}
#define glOrtho(left, right, bottom, top, near_val, far_val) glOrthoError(left, right, bottom, top, near_val, far_val, __FILE__, __LINE__)

inline void glFrustumError(GLdouble  left, GLdouble  right, GLdouble  bottom, GLdouble  top, GLdouble  near_val, GLdouble  far_val, const char *filename, int lineno)
{
  glFrustum(left, right, bottom, top, near_val, far_val);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFrustum]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFrustum(left, right, bottom, top, near_val, far_val) glFrustumError(left, right, bottom, top, near_val, far_val, __FILE__, __LINE__)

inline void glViewportError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glViewport(x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glViewport]: %s\n", filename, lineno, gluErrorString(error));
}
#define glViewport(x, y, width, height) glViewportError(x, y, width, height, __FILE__, __LINE__)

inline void glPushMatrixError(const char *filename, int lineno)
{
  glPushMatrix();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushMatrix]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushMatrix() glPushMatrixError(__FILE__, __LINE__)

inline void glPopMatrixError(const char *filename, int lineno)
{
  glPopMatrix();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopMatrix]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopMatrix() glPopMatrixError(__FILE__, __LINE__)

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

inline void glDeleteListsError(GLuint  list, GLsizei  range, const char *filename, int lineno)
{
  glDeleteLists(list, range);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDeleteLists]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDeleteLists(list, range) glDeleteListsError(list, range, __FILE__, __LINE__)

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

inline void glNewListError(GLuint  list, GLenum  mode, const char *filename, int lineno)
{
  glNewList(list, mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNewList]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNewList(list, mode) glNewListError(list, mode, __FILE__, __LINE__)

inline void glEndListError(const char *filename, int lineno)
{
  glEndList();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEndList]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEndList() glEndListError(__FILE__, __LINE__)

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

inline void glListBaseError(GLuint  base, const char *filename, int lineno)
{
  glListBase(base);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glListBase]: %s\n", filename, lineno, gluErrorString(error));
}
#define glListBase(base) glListBaseError(base, __FILE__, __LINE__)

inline void glBeginError(GLenum  mode, const char *filename, int lineno)
{
  glBegin(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBegin]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBegin(mode) glBeginError(mode, __FILE__, __LINE__)

inline void glEndError(const char *filename, int lineno)
{
  glEnd();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEnd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEnd() glEndError(__FILE__, __LINE__)

inline void glVertex2dError(GLdouble  x, GLdouble  y, const char *filename, int lineno)
{
  glVertex2d(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2d(x, y) glVertex2dError(x, y, __FILE__, __LINE__)

inline void glVertex2fError(GLfloat  x, GLfloat  y, const char *filename, int lineno)
{
  glVertex2f(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2f(x, y) glVertex2fError(x, y, __FILE__, __LINE__)

inline void glVertex2iError(GLint  x, GLint  y, const char *filename, int lineno)
{
  glVertex2i(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2i(x, y) glVertex2iError(x, y, __FILE__, __LINE__)

inline void glVertex2sError(GLshort  x, GLshort  y, const char *filename, int lineno)
{
  glVertex2s(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2s(x, y) glVertex2sError(x, y, __FILE__, __LINE__)

inline void glVertex3dError(GLdouble  x, GLdouble  y, GLdouble  z, const char *filename, int lineno)
{
  glVertex3d(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3d(x, y, z) glVertex3dError(x, y, z, __FILE__, __LINE__)

inline void glVertex3fError(GLfloat  x, GLfloat  y, GLfloat  z, const char *filename, int lineno)
{
  glVertex3f(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3f(x, y, z) glVertex3fError(x, y, z, __FILE__, __LINE__)

inline void glVertex3iError(GLint  x, GLint  y, GLint  z, const char *filename, int lineno)
{
  glVertex3i(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3i(x, y, z) glVertex3iError(x, y, z, __FILE__, __LINE__)

inline void glVertex3sError(GLshort  x, GLshort  y, GLshort  z, const char *filename, int lineno)
{
  glVertex3s(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3s(x, y, z) glVertex3sError(x, y, z, __FILE__, __LINE__)

inline void glVertex4dError(GLdouble  x, GLdouble  y, GLdouble  z, GLdouble  w, const char *filename, int lineno)
{
  glVertex4d(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4d(x, y, z, w) glVertex4dError(x, y, z, w, __FILE__, __LINE__)

inline void glVertex4fError(GLfloat  x, GLfloat  y, GLfloat  z, GLfloat  w, const char *filename, int lineno)
{
  glVertex4f(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4f(x, y, z, w) glVertex4fError(x, y, z, w, __FILE__, __LINE__)

inline void glVertex4iError(GLint  x, GLint  y, GLint  z, GLint  w, const char *filename, int lineno)
{
  glVertex4i(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4i(x, y, z, w) glVertex4iError(x, y, z, w, __FILE__, __LINE__)

inline void glVertex4sError(GLshort  x, GLshort  y, GLshort  z, GLshort  w, const char *filename, int lineno)
{
  glVertex4s(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4s(x, y, z, w) glVertex4sError(x, y, z, w, __FILE__, __LINE__)

inline void glVertex2dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glVertex2dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2dv(v) glVertex2dvError(v, __FILE__, __LINE__)

inline void glVertex2fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glVertex2fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2fv(v) glVertex2fvError(v, __FILE__, __LINE__)

inline void glVertex2ivError(const  GLint * v, const char *filename, int lineno)
{
  glVertex2iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2iv(v) glVertex2ivError(v, __FILE__, __LINE__)

inline void glVertex2svError(const  GLshort * v, const char *filename, int lineno)
{
  glVertex2sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex2sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex2sv(v) glVertex2svError(v, __FILE__, __LINE__)

inline void glVertex3dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glVertex3dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3dv(v) glVertex3dvError(v, __FILE__, __LINE__)

inline void glVertex3fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glVertex3fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3fv(v) glVertex3fvError(v, __FILE__, __LINE__)

inline void glVertex3ivError(const  GLint * v, const char *filename, int lineno)
{
  glVertex3iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3iv(v) glVertex3ivError(v, __FILE__, __LINE__)

inline void glVertex3svError(const  GLshort * v, const char *filename, int lineno)
{
  glVertex3sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex3sv(v) glVertex3svError(v, __FILE__, __LINE__)

inline void glVertex4dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glVertex4dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4dv(v) glVertex4dvError(v, __FILE__, __LINE__)

inline void glVertex4fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glVertex4fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4fv(v) glVertex4fvError(v, __FILE__, __LINE__)

inline void glVertex4ivError(const  GLint * v, const char *filename, int lineno)
{
  glVertex4iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4iv(v) glVertex4ivError(v, __FILE__, __LINE__)

inline void glVertex4svError(const  GLshort * v, const char *filename, int lineno)
{
  glVertex4sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertex4sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertex4sv(v) glVertex4svError(v, __FILE__, __LINE__)

inline void glNormal3bError(GLbyte  nx, GLbyte  ny, GLbyte  nz, const char *filename, int lineno)
{
  glNormal3b(nx, ny, nz);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3b]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3b(nx, ny, nz) glNormal3bError(nx, ny, nz, __FILE__, __LINE__)

inline void glNormal3dError(GLdouble  nx, GLdouble  ny, GLdouble  nz, const char *filename, int lineno)
{
  glNormal3d(nx, ny, nz);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3d(nx, ny, nz) glNormal3dError(nx, ny, nz, __FILE__, __LINE__)

inline void glNormal3fError(GLfloat  nx, GLfloat  ny, GLfloat  nz, const char *filename, int lineno)
{
  glNormal3f(nx, ny, nz);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3f(nx, ny, nz) glNormal3fError(nx, ny, nz, __FILE__, __LINE__)

inline void glNormal3iError(GLint  nx, GLint  ny, GLint  nz, const char *filename, int lineno)
{
  glNormal3i(nx, ny, nz);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3i(nx, ny, nz) glNormal3iError(nx, ny, nz, __FILE__, __LINE__)

inline void glNormal3sError(GLshort  nx, GLshort  ny, GLshort  nz, const char *filename, int lineno)
{
  glNormal3s(nx, ny, nz);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3s(nx, ny, nz) glNormal3sError(nx, ny, nz, __FILE__, __LINE__)

inline void glNormal3bvError(const  GLbyte * v, const char *filename, int lineno)
{
  glNormal3bv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3bv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3bv(v) glNormal3bvError(v, __FILE__, __LINE__)

inline void glNormal3dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glNormal3dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3dv(v) glNormal3dvError(v, __FILE__, __LINE__)

inline void glNormal3fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glNormal3fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3fv(v) glNormal3fvError(v, __FILE__, __LINE__)

inline void glNormal3ivError(const  GLint * v, const char *filename, int lineno)
{
  glNormal3iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3iv(v) glNormal3ivError(v, __FILE__, __LINE__)

inline void glNormal3svError(const  GLshort * v, const char *filename, int lineno)
{
  glNormal3sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormal3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormal3sv(v) glNormal3svError(v, __FILE__, __LINE__)

inline void glIndexdError(GLdouble  c, const char *filename, int lineno)
{
  glIndexd(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexd(c) glIndexdError(c, __FILE__, __LINE__)

inline void glIndexfError(GLfloat  c, const char *filename, int lineno)
{
  glIndexf(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexf(c) glIndexfError(c, __FILE__, __LINE__)

inline void glIndexiError(GLint  c, const char *filename, int lineno)
{
  glIndexi(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexi]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexi(c) glIndexiError(c, __FILE__, __LINE__)

inline void glIndexsError(GLshort  c, const char *filename, int lineno)
{
  glIndexs(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexs]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexs(c) glIndexsError(c, __FILE__, __LINE__)

inline void glIndexubError(GLubyte  c, const char *filename, int lineno)
{
  glIndexub(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexub]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexub(c) glIndexubError(c, __FILE__, __LINE__)

inline void glIndexdvError(const  GLdouble * c, const char *filename, int lineno)
{
  glIndexdv(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexdv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexdv(c) glIndexdvError(c, __FILE__, __LINE__)

inline void glIndexfvError(const  GLfloat * c, const char *filename, int lineno)
{
  glIndexfv(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexfv(c) glIndexfvError(c, __FILE__, __LINE__)

inline void glIndexivError(const  GLint * c, const char *filename, int lineno)
{
  glIndexiv(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexiv(c) glIndexivError(c, __FILE__, __LINE__)

inline void glIndexsvError(const  GLshort * c, const char *filename, int lineno)
{
  glIndexsv(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexsv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexsv(c) glIndexsvError(c, __FILE__, __LINE__)

inline void glIndexubvError(const  GLubyte * c, const char *filename, int lineno)
{
  glIndexubv(c);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexubv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexubv(c) glIndexubvError(c, __FILE__, __LINE__)

inline void glColor3bError(GLbyte  red, GLbyte  green, GLbyte  blue, const char *filename, int lineno)
{
  glColor3b(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3b]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3b(red, green, blue) glColor3bError(red, green, blue, __FILE__, __LINE__)

inline void glColor3dError(GLdouble  red, GLdouble  green, GLdouble  blue, const char *filename, int lineno)
{
  glColor3d(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3d(red, green, blue) glColor3dError(red, green, blue, __FILE__, __LINE__)

inline void glColor3fError(GLfloat  red, GLfloat  green, GLfloat  blue, const char *filename, int lineno)
{
  glColor3f(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3f(red, green, blue) glColor3fError(red, green, blue, __FILE__, __LINE__)

inline void glColor3iError(GLint  red, GLint  green, GLint  blue, const char *filename, int lineno)
{
  glColor3i(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3i(red, green, blue) glColor3iError(red, green, blue, __FILE__, __LINE__)

inline void glColor3sError(GLshort  red, GLshort  green, GLshort  blue, const char *filename, int lineno)
{
  glColor3s(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3s(red, green, blue) glColor3sError(red, green, blue, __FILE__, __LINE__)

inline void glColor3ubError(GLubyte  red, GLubyte  green, GLubyte  blue, const char *filename, int lineno)
{
  glColor3ub(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3ub]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3ub(red, green, blue) glColor3ubError(red, green, blue, __FILE__, __LINE__)

inline void glColor3uiError(GLuint  red, GLuint  green, GLuint  blue, const char *filename, int lineno)
{
  glColor3ui(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3ui]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3ui(red, green, blue) glColor3uiError(red, green, blue, __FILE__, __LINE__)

inline void glColor3usError(GLushort  red, GLushort  green, GLushort  blue, const char *filename, int lineno)
{
  glColor3us(red, green, blue);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3us]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3us(red, green, blue) glColor3usError(red, green, blue, __FILE__, __LINE__)

inline void glColor4bError(GLbyte  red, GLbyte  green, GLbyte  blue, GLbyte  alpha, const char *filename, int lineno)
{
  glColor4b(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4b]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4b(red, green, blue, alpha) glColor4bError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor4dError(GLdouble  red, GLdouble  green, GLdouble  blue, GLdouble  alpha, const char *filename, int lineno)
{
  glColor4d(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4d(red, green, blue, alpha) glColor4dError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor4fError(GLfloat  red, GLfloat  green, GLfloat  blue, GLfloat  alpha, const char *filename, int lineno)
{
  glColor4f(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4f(red, green, blue, alpha) glColor4fError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor4iError(GLint  red, GLint  green, GLint  blue, GLint  alpha, const char *filename, int lineno)
{
  glColor4i(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4i(red, green, blue, alpha) glColor4iError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor4sError(GLshort  red, GLshort  green, GLshort  blue, GLshort  alpha, const char *filename, int lineno)
{
  glColor4s(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4s(red, green, blue, alpha) glColor4sError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor4ubError(GLubyte  red, GLubyte  green, GLubyte  blue, GLubyte  alpha, const char *filename, int lineno)
{
  glColor4ub(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4ub]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4ub(red, green, blue, alpha) glColor4ubError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor4uiError(GLuint  red, GLuint  green, GLuint  blue, GLuint  alpha, const char *filename, int lineno)
{
  glColor4ui(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4ui]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4ui(red, green, blue, alpha) glColor4uiError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor4usError(GLushort  red, GLushort  green, GLushort  blue, GLushort  alpha, const char *filename, int lineno)
{
  glColor4us(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4us]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4us(red, green, blue, alpha) glColor4usError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glColor3bvError(const  GLbyte * v, const char *filename, int lineno)
{
  glColor3bv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3bv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3bv(v) glColor3bvError(v, __FILE__, __LINE__)

inline void glColor3dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glColor3dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3dv(v) glColor3dvError(v, __FILE__, __LINE__)

inline void glColor3fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glColor3fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3fv(v) glColor3fvError(v, __FILE__, __LINE__)

inline void glColor3ivError(const  GLint * v, const char *filename, int lineno)
{
  glColor3iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3iv(v) glColor3ivError(v, __FILE__, __LINE__)

inline void glColor3svError(const  GLshort * v, const char *filename, int lineno)
{
  glColor3sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3sv(v) glColor3svError(v, __FILE__, __LINE__)

inline void glColor3ubvError(const  GLubyte * v, const char *filename, int lineno)
{
  glColor3ubv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3ubv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3ubv(v) glColor3ubvError(v, __FILE__, __LINE__)

inline void glColor3uivError(const  GLuint * v, const char *filename, int lineno)
{
  glColor3uiv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3uiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3uiv(v) glColor3uivError(v, __FILE__, __LINE__)

inline void glColor3usvError(const  GLushort * v, const char *filename, int lineno)
{
  glColor3usv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor3usv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor3usv(v) glColor3usvError(v, __FILE__, __LINE__)

inline void glColor4bvError(const  GLbyte * v, const char *filename, int lineno)
{
  glColor4bv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4bv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4bv(v) glColor4bvError(v, __FILE__, __LINE__)

inline void glColor4dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glColor4dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4dv(v) glColor4dvError(v, __FILE__, __LINE__)

inline void glColor4fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glColor4fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4fv(v) glColor4fvError(v, __FILE__, __LINE__)

inline void glColor4ivError(const  GLint * v, const char *filename, int lineno)
{
  glColor4iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4iv(v) glColor4ivError(v, __FILE__, __LINE__)

inline void glColor4svError(const  GLshort * v, const char *filename, int lineno)
{
  glColor4sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4sv(v) glColor4svError(v, __FILE__, __LINE__)

inline void glColor4ubvError(const  GLubyte * v, const char *filename, int lineno)
{
  glColor4ubv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4ubv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4ubv(v) glColor4ubvError(v, __FILE__, __LINE__)

inline void glColor4uivError(const  GLuint * v, const char *filename, int lineno)
{
  glColor4uiv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4uiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4uiv(v) glColor4uivError(v, __FILE__, __LINE__)

inline void glColor4usvError(const  GLushort * v, const char *filename, int lineno)
{
  glColor4usv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColor4usv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColor4usv(v) glColor4usvError(v, __FILE__, __LINE__)

inline void glTexCoord1dError(GLdouble  s, const char *filename, int lineno)
{
  glTexCoord1d(s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1d(s) glTexCoord1dError(s, __FILE__, __LINE__)

inline void glTexCoord1fError(GLfloat  s, const char *filename, int lineno)
{
  glTexCoord1f(s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1f(s) glTexCoord1fError(s, __FILE__, __LINE__)

inline void glTexCoord1iError(GLint  s, const char *filename, int lineno)
{
  glTexCoord1i(s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1i(s) glTexCoord1iError(s, __FILE__, __LINE__)

inline void glTexCoord1sError(GLshort  s, const char *filename, int lineno)
{
  glTexCoord1s(s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1s(s) glTexCoord1sError(s, __FILE__, __LINE__)

inline void glTexCoord2dError(GLdouble  s, GLdouble  t, const char *filename, int lineno)
{
  glTexCoord2d(s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2d(s, t) glTexCoord2dError(s, t, __FILE__, __LINE__)

inline void glTexCoord2fError(GLfloat  s, GLfloat  t, const char *filename, int lineno)
{
  glTexCoord2f(s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2f(s, t) glTexCoord2fError(s, t, __FILE__, __LINE__)

inline void glTexCoord2iError(GLint  s, GLint  t, const char *filename, int lineno)
{
  glTexCoord2i(s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2i(s, t) glTexCoord2iError(s, t, __FILE__, __LINE__)

inline void glTexCoord2sError(GLshort  s, GLshort  t, const char *filename, int lineno)
{
  glTexCoord2s(s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2s(s, t) glTexCoord2sError(s, t, __FILE__, __LINE__)

inline void glTexCoord3dError(GLdouble  s, GLdouble  t, GLdouble  r, const char *filename, int lineno)
{
  glTexCoord3d(s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3d(s, t, r) glTexCoord3dError(s, t, r, __FILE__, __LINE__)

inline void glTexCoord3fError(GLfloat  s, GLfloat  t, GLfloat  r, const char *filename, int lineno)
{
  glTexCoord3f(s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3f(s, t, r) glTexCoord3fError(s, t, r, __FILE__, __LINE__)

inline void glTexCoord3iError(GLint  s, GLint  t, GLint  r, const char *filename, int lineno)
{
  glTexCoord3i(s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3i(s, t, r) glTexCoord3iError(s, t, r, __FILE__, __LINE__)

inline void glTexCoord3sError(GLshort  s, GLshort  t, GLshort  r, const char *filename, int lineno)
{
  glTexCoord3s(s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3s(s, t, r) glTexCoord3sError(s, t, r, __FILE__, __LINE__)

inline void glTexCoord4dError(GLdouble  s, GLdouble  t, GLdouble  r, GLdouble  q, const char *filename, int lineno)
{
  glTexCoord4d(s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4d(s, t, r, q) glTexCoord4dError(s, t, r, q, __FILE__, __LINE__)

inline void glTexCoord4fError(GLfloat  s, GLfloat  t, GLfloat  r, GLfloat  q, const char *filename, int lineno)
{
  glTexCoord4f(s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4f(s, t, r, q) glTexCoord4fError(s, t, r, q, __FILE__, __LINE__)

inline void glTexCoord4iError(GLint  s, GLint  t, GLint  r, GLint  q, const char *filename, int lineno)
{
  glTexCoord4i(s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4i(s, t, r, q) glTexCoord4iError(s, t, r, q, __FILE__, __LINE__)

inline void glTexCoord4sError(GLshort  s, GLshort  t, GLshort  r, GLshort  q, const char *filename, int lineno)
{
  glTexCoord4s(s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4s(s, t, r, q) glTexCoord4sError(s, t, r, q, __FILE__, __LINE__)

inline void glTexCoord1dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glTexCoord1dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1dv(v) glTexCoord1dvError(v, __FILE__, __LINE__)

inline void glTexCoord1fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glTexCoord1fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1fv(v) glTexCoord1fvError(v, __FILE__, __LINE__)

inline void glTexCoord1ivError(const  GLint * v, const char *filename, int lineno)
{
  glTexCoord1iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1iv(v) glTexCoord1ivError(v, __FILE__, __LINE__)

inline void glTexCoord1svError(const  GLshort * v, const char *filename, int lineno)
{
  glTexCoord1sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord1sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord1sv(v) glTexCoord1svError(v, __FILE__, __LINE__)

inline void glTexCoord2dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glTexCoord2dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2dv(v) glTexCoord2dvError(v, __FILE__, __LINE__)

inline void glTexCoord2fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glTexCoord2fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2fv(v) glTexCoord2fvError(v, __FILE__, __LINE__)

inline void glTexCoord2ivError(const  GLint * v, const char *filename, int lineno)
{
  glTexCoord2iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2iv(v) glTexCoord2ivError(v, __FILE__, __LINE__)

inline void glTexCoord2svError(const  GLshort * v, const char *filename, int lineno)
{
  glTexCoord2sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord2sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord2sv(v) glTexCoord2svError(v, __FILE__, __LINE__)

inline void glTexCoord3dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glTexCoord3dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3dv(v) glTexCoord3dvError(v, __FILE__, __LINE__)

inline void glTexCoord3fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glTexCoord3fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3fv(v) glTexCoord3fvError(v, __FILE__, __LINE__)

inline void glTexCoord3ivError(const  GLint * v, const char *filename, int lineno)
{
  glTexCoord3iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3iv(v) glTexCoord3ivError(v, __FILE__, __LINE__)

inline void glTexCoord3svError(const  GLshort * v, const char *filename, int lineno)
{
  glTexCoord3sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord3sv(v) glTexCoord3svError(v, __FILE__, __LINE__)

inline void glTexCoord4dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glTexCoord4dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4dv(v) glTexCoord4dvError(v, __FILE__, __LINE__)

inline void glTexCoord4fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glTexCoord4fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4fv(v) glTexCoord4fvError(v, __FILE__, __LINE__)

inline void glTexCoord4ivError(const  GLint * v, const char *filename, int lineno)
{
  glTexCoord4iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4iv(v) glTexCoord4ivError(v, __FILE__, __LINE__)

inline void glTexCoord4svError(const  GLshort * v, const char *filename, int lineno)
{
  glTexCoord4sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoord4sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoord4sv(v) glTexCoord4svError(v, __FILE__, __LINE__)

inline void glRasterPos2dError(GLdouble  x, GLdouble  y, const char *filename, int lineno)
{
  glRasterPos2d(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2d(x, y) glRasterPos2dError(x, y, __FILE__, __LINE__)

inline void glRasterPos2fError(GLfloat  x, GLfloat  y, const char *filename, int lineno)
{
  glRasterPos2f(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2f(x, y) glRasterPos2fError(x, y, __FILE__, __LINE__)

inline void glRasterPos2iError(GLint  x, GLint  y, const char *filename, int lineno)
{
  glRasterPos2i(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2i(x, y) glRasterPos2iError(x, y, __FILE__, __LINE__)

inline void glRasterPos2sError(GLshort  x, GLshort  y, const char *filename, int lineno)
{
  glRasterPos2s(x, y);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2s(x, y) glRasterPos2sError(x, y, __FILE__, __LINE__)

inline void glRasterPos3dError(GLdouble  x, GLdouble  y, GLdouble  z, const char *filename, int lineno)
{
  glRasterPos3d(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3d(x, y, z) glRasterPos3dError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos3fError(GLfloat  x, GLfloat  y, GLfloat  z, const char *filename, int lineno)
{
  glRasterPos3f(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3f(x, y, z) glRasterPos3fError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos3iError(GLint  x, GLint  y, GLint  z, const char *filename, int lineno)
{
  glRasterPos3i(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3i(x, y, z) glRasterPos3iError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos3sError(GLshort  x, GLshort  y, GLshort  z, const char *filename, int lineno)
{
  glRasterPos3s(x, y, z);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3s(x, y, z) glRasterPos3sError(x, y, z, __FILE__, __LINE__)

inline void glRasterPos4dError(GLdouble  x, GLdouble  y, GLdouble  z, GLdouble  w, const char *filename, int lineno)
{
  glRasterPos4d(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4d(x, y, z, w) glRasterPos4dError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos4fError(GLfloat  x, GLfloat  y, GLfloat  z, GLfloat  w, const char *filename, int lineno)
{
  glRasterPos4f(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4f(x, y, z, w) glRasterPos4fError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos4iError(GLint  x, GLint  y, GLint  z, GLint  w, const char *filename, int lineno)
{
  glRasterPos4i(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4i(x, y, z, w) glRasterPos4iError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos4sError(GLshort  x, GLshort  y, GLshort  z, GLshort  w, const char *filename, int lineno)
{
  glRasterPos4s(x, y, z, w);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4s(x, y, z, w) glRasterPos4sError(x, y, z, w, __FILE__, __LINE__)

inline void glRasterPos2dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glRasterPos2dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2dv(v) glRasterPos2dvError(v, __FILE__, __LINE__)

inline void glRasterPos2fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glRasterPos2fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2fv(v) glRasterPos2fvError(v, __FILE__, __LINE__)

inline void glRasterPos2ivError(const  GLint * v, const char *filename, int lineno)
{
  glRasterPos2iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2iv(v) glRasterPos2ivError(v, __FILE__, __LINE__)

inline void glRasterPos2svError(const  GLshort * v, const char *filename, int lineno)
{
  glRasterPos2sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos2sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos2sv(v) glRasterPos2svError(v, __FILE__, __LINE__)

inline void glRasterPos3dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glRasterPos3dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3dv(v) glRasterPos3dvError(v, __FILE__, __LINE__)

inline void glRasterPos3fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glRasterPos3fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3fv(v) glRasterPos3fvError(v, __FILE__, __LINE__)

inline void glRasterPos3ivError(const  GLint * v, const char *filename, int lineno)
{
  glRasterPos3iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3iv(v) glRasterPos3ivError(v, __FILE__, __LINE__)

inline void glRasterPos3svError(const  GLshort * v, const char *filename, int lineno)
{
  glRasterPos3sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos3sv(v) glRasterPos3svError(v, __FILE__, __LINE__)

inline void glRasterPos4dvError(const  GLdouble * v, const char *filename, int lineno)
{
  glRasterPos4dv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4dv(v) glRasterPos4dvError(v, __FILE__, __LINE__)

inline void glRasterPos4fvError(const  GLfloat * v, const char *filename, int lineno)
{
  glRasterPos4fv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4fv(v) glRasterPos4fvError(v, __FILE__, __LINE__)

inline void glRasterPos4ivError(const  GLint * v, const char *filename, int lineno)
{
  glRasterPos4iv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4iv(v) glRasterPos4ivError(v, __FILE__, __LINE__)

inline void glRasterPos4svError(const  GLshort * v, const char *filename, int lineno)
{
  glRasterPos4sv(v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRasterPos4sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRasterPos4sv(v) glRasterPos4svError(v, __FILE__, __LINE__)

inline void glRectdError(GLdouble  x1, GLdouble  y1, GLdouble  x2, GLdouble  y2, const char *filename, int lineno)
{
  glRectd(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectd(x1, y1, x2, y2) glRectdError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectfError(GLfloat  x1, GLfloat  y1, GLfloat  x2, GLfloat  y2, const char *filename, int lineno)
{
  glRectf(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectf(x1, y1, x2, y2) glRectfError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectiError(GLint  x1, GLint  y1, GLint  x2, GLint  y2, const char *filename, int lineno)
{
  glRecti(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRecti]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRecti(x1, y1, x2, y2) glRectiError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectsError(GLshort  x1, GLshort  y1, GLshort  x2, GLshort  y2, const char *filename, int lineno)
{
  glRects(x1, y1, x2, y2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRects]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRects(x1, y1, x2, y2) glRectsError(x1, y1, x2, y2, __FILE__, __LINE__)

inline void glRectdvError(const  GLdouble * v1, const  GLdouble * v2, const char *filename, int lineno)
{
  glRectdv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectdv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectdv(v1, v2) glRectdvError(v1, v2, __FILE__, __LINE__)

inline void glRectfvError(const  GLfloat * v1, const  GLfloat * v2, const char *filename, int lineno)
{
  glRectfv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectfv(v1, v2) glRectfvError(v1, v2, __FILE__, __LINE__)

inline void glRectivError(const  GLint * v1, const  GLint * v2, const char *filename, int lineno)
{
  glRectiv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectiv(v1, v2) glRectivError(v1, v2, __FILE__, __LINE__)

inline void glRectsvError(const  GLshort * v1, const  GLshort * v2, const char *filename, int lineno)
{
  glRectsv(v1, v2);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glRectsv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glRectsv(v1, v2) glRectsvError(v1, v2, __FILE__, __LINE__)

inline void glVertexPointerError(GLint  size, GLenum  type, GLsizei  stride, const  GLvoid * ptr, const char *filename, int lineno)
{
  glVertexPointer(size, type, stride, ptr);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glVertexPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glVertexPointer(size, type, stride, ptr) glVertexPointerError(size, type, stride, ptr, __FILE__, __LINE__)

inline void glNormalPointerError(GLenum  type, GLsizei  stride, const  GLvoid * ptr, const char *filename, int lineno)
{
  glNormalPointer(type, stride, ptr);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glNormalPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glNormalPointer(type, stride, ptr) glNormalPointerError(type, stride, ptr, __FILE__, __LINE__)

inline void glColorPointerError(GLint  size, GLenum  type, GLsizei  stride, const  GLvoid * ptr, const char *filename, int lineno)
{
  glColorPointer(size, type, stride, ptr);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorPointer(size, type, stride, ptr) glColorPointerError(size, type, stride, ptr, __FILE__, __LINE__)

inline void glIndexPointerError(GLenum  type, GLsizei  stride, const  GLvoid * ptr, const char *filename, int lineno)
{
  glIndexPointer(type, stride, ptr);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glIndexPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glIndexPointer(type, stride, ptr) glIndexPointerError(type, stride, ptr, __FILE__, __LINE__)

inline void glTexCoordPointerError(GLint  size, GLenum  type, GLsizei  stride, const  GLvoid * ptr, const char *filename, int lineno)
{
  glTexCoordPointer(size, type, stride, ptr);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexCoordPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexCoordPointer(size, type, stride, ptr) glTexCoordPointerError(size, type, stride, ptr, __FILE__, __LINE__)

inline void glEdgeFlagPointerError(GLsizei  stride, const  GLvoid * ptr, const char *filename, int lineno)
{
  glEdgeFlagPointer(stride, ptr);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEdgeFlagPointer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEdgeFlagPointer(stride, ptr) glEdgeFlagPointerError(stride, ptr, __FILE__, __LINE__)

inline void glGetPointervError(GLenum  pname, GLvoid ** params, const char *filename, int lineno)
{
  glGetPointerv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetPointerv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetPointerv(pname, params) glGetPointervError(pname, params, __FILE__, __LINE__)

inline void glArrayElementError(GLint  i, const char *filename, int lineno)
{
  glArrayElement(i);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glArrayElement]: %s\n", filename, lineno, gluErrorString(error));
}
#define glArrayElement(i) glArrayElementError(i, __FILE__, __LINE__)

inline void glDrawArraysError(GLenum  mode, GLint  first, GLsizei  count, const char *filename, int lineno)
{
  glDrawArrays(mode, first, count);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawArrays]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawArrays(mode, first, count) glDrawArraysError(mode, first, count, __FILE__, __LINE__)

inline void glDrawElementsError(GLenum  mode, GLsizei  count, GLenum  type, const  GLvoid * indices, const char *filename, int lineno)
{
  glDrawElements(mode, count, type, indices);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawElements]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawElements(mode, count, type, indices) glDrawElementsError(mode, count, type, indices, __FILE__, __LINE__)

inline void glInterleavedArraysError(GLenum  format, GLsizei  stride, const  GLvoid * pointer, const char *filename, int lineno)
{
  glInterleavedArrays(format, stride, pointer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glInterleavedArrays]: %s\n", filename, lineno, gluErrorString(error));
}
#define glInterleavedArrays(format, stride, pointer) glInterleavedArraysError(format, stride, pointer, __FILE__, __LINE__)

inline void glShadeModelError(GLenum  mode, const char *filename, int lineno)
{
  glShadeModel(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glShadeModel]: %s\n", filename, lineno, gluErrorString(error));
}
#define glShadeModel(mode) glShadeModelError(mode, __FILE__, __LINE__)

inline void glLightfError(GLenum  light, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glLightf(light, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightf(light, pname, param) glLightfError(light, pname, param, __FILE__, __LINE__)

inline void glLightiError(GLenum  light, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glLighti(light, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLighti]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLighti(light, pname, param) glLightiError(light, pname, param, __FILE__, __LINE__)

inline void glLightfvError(GLenum  light, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glLightfv(light, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightfv(light, pname, params) glLightfvError(light, pname, params, __FILE__, __LINE__)

inline void glLightivError(GLenum  light, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glLightiv(light, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightiv(light, pname, params) glLightivError(light, pname, params, __FILE__, __LINE__)

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

inline void glLightModelfError(GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glLightModelf(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModelf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModelf(pname, param) glLightModelfError(pname, param, __FILE__, __LINE__)

inline void glLightModeliError(GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glLightModeli(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModeli]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModeli(pname, param) glLightModeliError(pname, param, __FILE__, __LINE__)

inline void glLightModelfvError(GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glLightModelfv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModelfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModelfv(pname, params) glLightModelfvError(pname, params, __FILE__, __LINE__)

inline void glLightModelivError(GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glLightModeliv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLightModeliv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLightModeliv(pname, params) glLightModelivError(pname, params, __FILE__, __LINE__)

inline void glMaterialfError(GLenum  face, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glMaterialf(face, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMaterialf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMaterialf(face, pname, param) glMaterialfError(face, pname, param, __FILE__, __LINE__)

inline void glMaterialiError(GLenum  face, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glMateriali(face, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMateriali]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMateriali(face, pname, param) glMaterialiError(face, pname, param, __FILE__, __LINE__)

inline void glMaterialfvError(GLenum  face, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glMaterialfv(face, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMaterialfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMaterialfv(face, pname, params) glMaterialfvError(face, pname, params, __FILE__, __LINE__)

inline void glMaterialivError(GLenum  face, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glMaterialiv(face, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMaterialiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMaterialiv(face, pname, params) glMaterialivError(face, pname, params, __FILE__, __LINE__)

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

inline void glColorMaterialError(GLenum  face, GLenum  mode, const char *filename, int lineno)
{
  glColorMaterial(face, mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorMaterial]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorMaterial(face, mode) glColorMaterialError(face, mode, __FILE__, __LINE__)

inline void glPixelZoomError(GLfloat  xfactor, GLfloat  yfactor, const char *filename, int lineno)
{
  glPixelZoom(xfactor, yfactor);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelZoom]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelZoom(xfactor, yfactor) glPixelZoomError(xfactor, yfactor, __FILE__, __LINE__)

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

inline void glPixelMapfvError(GLenum  map, GLint  mapsize, const  GLfloat * values, const char *filename, int lineno)
{
  glPixelMapfv(map, mapsize, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelMapfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelMapfv(map, mapsize, values) glPixelMapfvError(map, mapsize, values, __FILE__, __LINE__)

inline void glPixelMapuivError(GLenum  map, GLint  mapsize, const  GLuint * values, const char *filename, int lineno)
{
  glPixelMapuiv(map, mapsize, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelMapuiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelMapuiv(map, mapsize, values) glPixelMapuivError(map, mapsize, values, __FILE__, __LINE__)

inline void glPixelMapusvError(GLenum  map, GLint  mapsize, const  GLushort * values, const char *filename, int lineno)
{
  glPixelMapusv(map, mapsize, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPixelMapusv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPixelMapusv(map, mapsize, values) glPixelMapusvError(map, mapsize, values, __FILE__, __LINE__)

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

inline void glBitmapError(GLsizei  width, GLsizei  height, GLfloat  xorig, GLfloat  yorig, GLfloat  xmove, GLfloat  ymove, const  GLubyte * bitmap, const char *filename, int lineno)
{
  glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBitmap]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap) glBitmapError(width, height, xorig, yorig, xmove, ymove, bitmap, __FILE__, __LINE__)

inline void glReadPixelsError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, GLvoid * pixels, const char *filename, int lineno)
{
  glReadPixels(x, y, width, height, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glReadPixels]: %s\n", filename, lineno, gluErrorString(error));
}
#define glReadPixels(x, y, width, height, format, type, pixels) glReadPixelsError(x, y, width, height, format, type, pixels, __FILE__, __LINE__)

inline void glDrawPixelsError(GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glDrawPixels(width, height, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawPixels]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawPixels(width, height, format, type, pixels) glDrawPixelsError(width, height, format, type, pixels, __FILE__, __LINE__)

inline void glCopyPixelsError(GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLenum  type, const char *filename, int lineno)
{
  glCopyPixels(x, y, width, height, type);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyPixels]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyPixels(x, y, width, height, type) glCopyPixelsError(x, y, width, height, type, __FILE__, __LINE__)

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

inline void glClearStencilError(GLint  s, const char *filename, int lineno)
{
  glClearStencil(s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClearStencil]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClearStencil(s) glClearStencilError(s, __FILE__, __LINE__)

inline void glTexGendError(GLenum  coord, GLenum  pname, GLdouble  param, const char *filename, int lineno)
{
  glTexGend(coord, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGend]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGend(coord, pname, param) glTexGendError(coord, pname, param, __FILE__, __LINE__)

inline void glTexGenfError(GLenum  coord, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glTexGenf(coord, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGenf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGenf(coord, pname, param) glTexGenfError(coord, pname, param, __FILE__, __LINE__)

inline void glTexGeniError(GLenum  coord, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glTexGeni(coord, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGeni]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGeni(coord, pname, param) glTexGeniError(coord, pname, param, __FILE__, __LINE__)

inline void glTexGendvError(GLenum  coord, GLenum  pname, const  GLdouble * params, const char *filename, int lineno)
{
  glTexGendv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGendv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGendv(coord, pname, params) glTexGendvError(coord, pname, params, __FILE__, __LINE__)

inline void glTexGenfvError(GLenum  coord, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glTexGenfv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGenfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGenfv(coord, pname, params) glTexGenfvError(coord, pname, params, __FILE__, __LINE__)

inline void glTexGenivError(GLenum  coord, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glTexGeniv(coord, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexGeniv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexGeniv(coord, pname, params) glTexGenivError(coord, pname, params, __FILE__, __LINE__)

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

inline void glTexEnvfError(GLenum  target, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glTexEnvf(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnvf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnvf(target, pname, param) glTexEnvfError(target, pname, param, __FILE__, __LINE__)

inline void glTexEnviError(GLenum  target, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glTexEnvi(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnvi]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnvi(target, pname, param) glTexEnviError(target, pname, param, __FILE__, __LINE__)

inline void glTexEnvfvError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glTexEnvfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnvfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnvfv(target, pname, params) glTexEnvfvError(target, pname, params, __FILE__, __LINE__)

inline void glTexEnvivError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glTexEnviv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexEnviv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexEnviv(target, pname, params) glTexEnvivError(target, pname, params, __FILE__, __LINE__)

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

inline void glTexParameterfError(GLenum  target, GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glTexParameterf(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameterf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameterf(target, pname, param) glTexParameterfError(target, pname, param, __FILE__, __LINE__)

inline void glTexParameteriError(GLenum  target, GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glTexParameteri(target, pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameteri]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameteri(target, pname, param) glTexParameteriError(target, pname, param, __FILE__, __LINE__)

inline void glTexParameterfvError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glTexParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameterfv(target, pname, params) glTexParameterfvError(target, pname, params, __FILE__, __LINE__)

inline void glTexParameterivError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glTexParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexParameteriv(target, pname, params) glTexParameterivError(target, pname, params, __FILE__, __LINE__)

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

inline void glTexImage1DError(GLenum  target, GLint  level, GLint  internalFormat, GLsizei  width, GLint  border, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexImage1D(target, level, internalFormat, width, border, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexImage1D(target, level, internalFormat, width, border, format, type, pixels) glTexImage1DError(target, level, internalFormat, width, border, format, type, pixels, __FILE__, __LINE__)

inline void glTexImage2DError(GLenum  target, GLint  level, GLint  internalFormat, GLsizei  width, GLsizei  height, GLint  border, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels) glTexImage2DError(target, level, internalFormat, width, height, border, format, type, pixels, __FILE__, __LINE__)

inline void glGetTexImageError(GLenum  target, GLint  level, GLenum  format, GLenum  type, GLvoid * pixels, const char *filename, int lineno)
{
  glGetTexImage(target, level, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetTexImage]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetTexImage(target, level, format, type, pixels) glGetTexImageError(target, level, format, type, pixels, __FILE__, __LINE__)

inline void glGenTexturesError(GLsizei  n, GLuint * textures, const char *filename, int lineno)
{
  glGenTextures(n, textures);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGenTextures]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGenTextures(n, textures) glGenTexturesError(n, textures, __FILE__, __LINE__)

inline void glDeleteTexturesError(GLsizei  n, const  GLuint * textures, const char *filename, int lineno)
{
  glDeleteTextures(n, textures);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDeleteTextures]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDeleteTextures(n, textures) glDeleteTexturesError(n, textures, __FILE__, __LINE__)

inline void glBindTextureError(GLenum  target, GLuint  texture, const char *filename, int lineno)
{
  glBindTexture(target, texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBindTexture]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBindTexture(target, texture) glBindTextureError(target, texture, __FILE__, __LINE__)

inline void glPrioritizeTexturesError(GLsizei  n, const  GLuint * textures, const  GLclampf * priorities, const char *filename, int lineno)
{
  glPrioritizeTextures(n, textures, priorities);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPrioritizeTextures]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPrioritizeTextures(n, textures, priorities) glPrioritizeTexturesError(n, textures, priorities, __FILE__, __LINE__)

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

inline void glCopyTexImage1DError(GLenum  target, GLint  level, GLenum  internalformat, GLint  x, GLint  y, GLsizei  width, GLint  border, const char *filename, int lineno)
{
  glCopyTexImage1D(target, level, internalformat, x, y, width, border);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexImage1D(target, level, internalformat, x, y, width, border) glCopyTexImage1DError(target, level, internalformat, x, y, width, border, __FILE__, __LINE__)

inline void glCopyTexImage2DError(GLenum  target, GLint  level, GLenum  internalformat, GLint  x, GLint  y, GLsizei  width, GLsizei  height, GLint  border, const char *filename, int lineno)
{
  glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexImage2D(target, level, internalformat, x, y, width, height, border) glCopyTexImage2DError(target, level, internalformat, x, y, width, height, border, __FILE__, __LINE__)

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

inline void glEvalCoord1dError(GLdouble  u, const char *filename, int lineno)
{
  glEvalCoord1d(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord1d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord1d(u) glEvalCoord1dError(u, __FILE__, __LINE__)

inline void glEvalCoord1fError(GLfloat  u, const char *filename, int lineno)
{
  glEvalCoord1f(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord1f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord1f(u) glEvalCoord1fError(u, __FILE__, __LINE__)

inline void glEvalCoord1dvError(const  GLdouble * u, const char *filename, int lineno)
{
  glEvalCoord1dv(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord1dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord1dv(u) glEvalCoord1dvError(u, __FILE__, __LINE__)

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

inline void glEvalCoord2fError(GLfloat  u, GLfloat  v, const char *filename, int lineno)
{
  glEvalCoord2f(u, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord2f(u, v) glEvalCoord2fError(u, v, __FILE__, __LINE__)

inline void glEvalCoord2dvError(const  GLdouble * u, const char *filename, int lineno)
{
  glEvalCoord2dv(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord2dv(u) glEvalCoord2dvError(u, __FILE__, __LINE__)

inline void glEvalCoord2fvError(const  GLfloat * u, const char *filename, int lineno)
{
  glEvalCoord2fv(u);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glEvalCoord2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glEvalCoord2fv(u) glEvalCoord2fvError(u, __FILE__, __LINE__)

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

inline void glFogfError(GLenum  pname, GLfloat  param, const char *filename, int lineno)
{
  glFogf(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogf(pname, param) glFogfError(pname, param, __FILE__, __LINE__)

inline void glFogiError(GLenum  pname, GLint  param, const char *filename, int lineno)
{
  glFogi(pname, param);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogi]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogi(pname, param) glFogiError(pname, param, __FILE__, __LINE__)

inline void glFogfvError(GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glFogfv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogfv(pname, params) glFogfvError(pname, params, __FILE__, __LINE__)

inline void glFogivError(GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glFogiv(pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFogiv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFogiv(pname, params) glFogivError(pname, params, __FILE__, __LINE__)

inline void glFeedbackBufferError(GLsizei  size, GLenum  type, GLfloat * buffer, const char *filename, int lineno)
{
  glFeedbackBuffer(size, type, buffer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glFeedbackBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glFeedbackBuffer(size, type, buffer) glFeedbackBufferError(size, type, buffer, __FILE__, __LINE__)

inline void glPassThroughError(GLfloat  token, const char *filename, int lineno)
{
  glPassThrough(token);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPassThrough]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPassThrough(token) glPassThroughError(token, __FILE__, __LINE__)

inline void glSelectBufferError(GLsizei  size, GLuint * buffer, const char *filename, int lineno)
{
  glSelectBuffer(size, buffer);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSelectBuffer]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSelectBuffer(size, buffer) glSelectBufferError(size, buffer, __FILE__, __LINE__)

inline void glInitNamesError(const char *filename, int lineno)
{
  glInitNames();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glInitNames]: %s\n", filename, lineno, gluErrorString(error));
}
#define glInitNames() glInitNamesError(__FILE__, __LINE__)

inline void glLoadNameError(GLuint  name, const char *filename, int lineno)
{
  glLoadName(name);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadName]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadName(name) glLoadNameError(name, __FILE__, __LINE__)

inline void glPushNameError(GLuint  name, const char *filename, int lineno)
{
  glPushName(name);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPushName]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPushName(name) glPushNameError(name, __FILE__, __LINE__)

inline void glPopNameError(const char *filename, int lineno)
{
  glPopName();
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glPopName]: %s\n", filename, lineno, gluErrorString(error));
}
#define glPopName() glPopNameError(__FILE__, __LINE__)

inline void glDrawRangeElementsError(GLenum  mode, GLuint  start, GLuint  end, GLsizei  count, GLenum  type, const  GLvoid * indices, const char *filename, int lineno)
{
  glDrawRangeElements(mode, start, end, count, type, indices);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glDrawRangeElements]: %s\n", filename, lineno, gluErrorString(error));
}
#define glDrawRangeElements(mode, start, end, count, type, indices) glDrawRangeElementsError(mode, start, end, count, type, indices, __FILE__, __LINE__)

inline void glTexImage3DError(GLenum  target, GLint  level, GLenum  internalFormat, GLsizei  width, GLsizei  height, GLsizei  depth, GLint  border, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels) glTexImage3DError(target, level, internalFormat, width, height, depth, border, format, type, pixels, __FILE__, __LINE__)

inline void glTexSubImage3DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLsizei  width, GLsizei  height, GLsizei  depth, GLenum  format, GLenum  type, const  GLvoid * pixels, const char *filename, int lineno)
{
  glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glTexSubImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) glTexSubImage3DError(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels, __FILE__, __LINE__)

inline void glCopyTexSubImage3DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLint  x, GLint  y, GLsizei  width, GLsizei  height, const char *filename, int lineno)
{
  glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCopyTexSubImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height) glCopyTexSubImage3DError(target, level, xoffset, yoffset, zoffset, x, y, width, height, __FILE__, __LINE__)

inline void glColorTableError(GLenum  target, GLenum  internalformat, GLsizei  width, GLenum  format, GLenum  type, const  GLvoid * table, const char *filename, int lineno)
{
  glColorTable(target, internalformat, width, format, type, table);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTable(target, internalformat, width, format, type, table) glColorTableError(target, internalformat, width, format, type, table, __FILE__, __LINE__)

inline void glColorSubTableError(GLenum  target, GLsizei  start, GLsizei  count, GLenum  format, GLenum  type, const  GLvoid * data, const char *filename, int lineno)
{
  glColorSubTable(target, start, count, format, type, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorSubTable]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorSubTable(target, start, count, format, type, data) glColorSubTableError(target, start, count, format, type, data, __FILE__, __LINE__)

inline void glColorTableParameterivError(GLenum  target, GLenum  pname, const  GLint * params, const char *filename, int lineno)
{
  glColorTableParameteriv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTableParameteriv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTableParameteriv(target, pname, params) glColorTableParameterivError(target, pname, params, __FILE__, __LINE__)

inline void glColorTableParameterfvError(GLenum  target, GLenum  pname, const  GLfloat * params, const char *filename, int lineno)
{
  glColorTableParameterfv(target, pname, params);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glColorTableParameterfv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glColorTableParameterfv(target, pname, params) glColorTableParameterfvError(target, pname, params, __FILE__, __LINE__)

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

inline void glBlendEquationError(GLenum  mode, const char *filename, int lineno)
{
  glBlendEquation(mode);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendEquation]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendEquation(mode) glBlendEquationError(mode, __FILE__, __LINE__)

inline void glBlendColorError(GLclampf  red, GLclampf  green, GLclampf  blue, GLclampf  alpha, const char *filename, int lineno)
{
  glBlendColor(red, green, blue, alpha);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glBlendColor]: %s\n", filename, lineno, gluErrorString(error));
}
#define glBlendColor(red, green, blue, alpha) glBlendColorError(red, green, blue, alpha, __FILE__, __LINE__)

inline void glHistogramError(GLenum  target, GLsizei  width, GLenum  internalformat, GLboolean  sink, const char *filename, int lineno)
{
  glHistogram(target, width, internalformat, sink);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glHistogram]: %s\n", filename, lineno, gluErrorString(error));
}
#define glHistogram(target, width, internalformat, sink) glHistogramError(target, width, internalformat, sink, __FILE__, __LINE__)

inline void glResetHistogramError(GLenum  target, const char *filename, int lineno)
{
  glResetHistogram(target);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glResetHistogram]: %s\n", filename, lineno, gluErrorString(error));
}
#define glResetHistogram(target) glResetHistogramError(target, __FILE__, __LINE__)

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

inline void glMinmaxError(GLenum  target, GLenum  internalformat, GLboolean  sink, const char *filename, int lineno)
{
  glMinmax(target, internalformat, sink);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMinmax]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMinmax(target, internalformat, sink) glMinmaxError(target, internalformat, sink, __FILE__, __LINE__)

inline void glResetMinmaxError(GLenum  target, const char *filename, int lineno)
{
  glResetMinmax(target);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glResetMinmax]: %s\n", filename, lineno, gluErrorString(error));
}
#define glResetMinmax(target) glResetMinmaxError(target, __FILE__, __LINE__)

inline void glGetMinmaxError(GLenum  target, GLboolean  reset, GLenum  format, GLenum  types, GLvoid * values, const char *filename, int lineno)
{
  glGetMinmax(target, reset, format, types, values);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetMinmax]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetMinmax(target, reset, format, types, values) glGetMinmaxError(target, reset, format, types, values, __FILE__, __LINE__)

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

inline void glSeparableFilter2DError(GLenum  target, GLenum  internalformat, GLsizei  width, GLsizei  height, GLenum  format, GLenum  type, const  GLvoid * row, const  GLvoid * column, const char *filename, int lineno)
{
  glSeparableFilter2D(target, internalformat, width, height, format, type, row, column);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSeparableFilter2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSeparableFilter2D(target, internalformat, width, height, format, type, row, column) glSeparableFilter2DError(target, internalformat, width, height, format, type, row, column, __FILE__, __LINE__)

inline void glGetSeparableFilterError(GLenum  target, GLenum  format, GLenum  type, GLvoid * row, GLvoid * column, GLvoid * span, const char *filename, int lineno)
{
  glGetSeparableFilter(target, format, type, row, column, span);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetSeparableFilter]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetSeparableFilter(target, format, type, row, column, span) glGetSeparableFilterError(target, format, type, row, column, span, __FILE__, __LINE__)

inline void glActiveTextureError(GLenum  texture, const char *filename, int lineno)
{
  glActiveTexture(texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glActiveTexture]: %s\n", filename, lineno, gluErrorString(error));
}
#define glActiveTexture(texture) glActiveTextureError(texture, __FILE__, __LINE__)

inline void glClientActiveTextureError(GLenum  texture, const char *filename, int lineno)
{
  glClientActiveTexture(texture);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glClientActiveTexture]: %s\n", filename, lineno, gluErrorString(error));
}
#define glClientActiveTexture(texture) glClientActiveTextureError(texture, __FILE__, __LINE__)

inline void glCompressedTexImage1DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLint  border, GLsizei  imageSize, const  GLvoid * data, const char *filename, int lineno)
{
  glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data) glCompressedTexImage1DError(target, level, internalformat, width, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexImage2DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLint  border, GLsizei  imageSize, const  GLvoid * data, const char *filename, int lineno)
{
  glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data) glCompressedTexImage2DError(target, level, internalformat, width, height, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexImage3DError(GLenum  target, GLint  level, GLenum  internalformat, GLsizei  width, GLsizei  height, GLsizei  depth, GLint  border, GLsizei  imageSize, const  GLvoid * data, const char *filename, int lineno)
{
  glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data) glCompressedTexImage3DError(target, level, internalformat, width, height, depth, border, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage1DError(GLenum  target, GLint  level, GLint  xoffset, GLsizei  width, GLenum  format, GLsizei  imageSize, const  GLvoid * data, const char *filename, int lineno)
{
  glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage1D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data) glCompressedTexSubImage1DError(target, level, xoffset, width, format, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage2DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLsizei  width, GLsizei  height, GLenum  format, GLsizei  imageSize, const  GLvoid * data, const char *filename, int lineno)
{
  glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage2D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data) glCompressedTexSubImage2DError(target, level, xoffset, yoffset, width, height, format, imageSize, data, __FILE__, __LINE__)

inline void glCompressedTexSubImage3DError(GLenum  target, GLint  level, GLint  xoffset, GLint  yoffset, GLint  zoffset, GLsizei  width, GLsizei  height, GLsizei  depth, GLenum  format, GLsizei  imageSize, const  GLvoid * data, const char *filename, int lineno)
{
  glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glCompressedTexSubImage3D]: %s\n", filename, lineno, gluErrorString(error));
}
#define glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) glCompressedTexSubImage3DError(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data, __FILE__, __LINE__)

inline void glGetCompressedTexImageError(GLenum  target, GLint  lod, GLvoid * img, const char *filename, int lineno)
{
  glGetCompressedTexImage(target, lod, img);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glGetCompressedTexImage]: %s\n", filename, lineno, gluErrorString(error));
}
#define glGetCompressedTexImage(target, lod, img) glGetCompressedTexImageError(target, lod, img, __FILE__, __LINE__)

inline void glMultiTexCoord1dError(GLenum  target, GLdouble  s, const char *filename, int lineno)
{
  glMultiTexCoord1d(target, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1d(target, s) glMultiTexCoord1dError(target, s, __FILE__, __LINE__)

inline void glMultiTexCoord1dvError(GLenum  target, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord1dv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1dv(target, v) glMultiTexCoord1dvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord1fError(GLenum  target, GLfloat  s, const char *filename, int lineno)
{
  glMultiTexCoord1f(target, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1f(target, s) glMultiTexCoord1fError(target, s, __FILE__, __LINE__)

inline void glMultiTexCoord1fvError(GLenum  target, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord1fv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1fv(target, v) glMultiTexCoord1fvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord1iError(GLenum  target, GLint  s, const char *filename, int lineno)
{
  glMultiTexCoord1i(target, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1i(target, s) glMultiTexCoord1iError(target, s, __FILE__, __LINE__)

inline void glMultiTexCoord1ivError(GLenum  target, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord1iv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1iv(target, v) glMultiTexCoord1ivError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord1sError(GLenum  target, GLshort  s, const char *filename, int lineno)
{
  glMultiTexCoord1s(target, s);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1s(target, s) glMultiTexCoord1sError(target, s, __FILE__, __LINE__)

inline void glMultiTexCoord1svError(GLenum  target, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord1sv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord1sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord1sv(target, v) glMultiTexCoord1svError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord2dError(GLenum  target, GLdouble  s, GLdouble  t, const char *filename, int lineno)
{
  glMultiTexCoord2d(target, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2d(target, s, t) glMultiTexCoord2dError(target, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2dvError(GLenum  target, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord2dv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2dv(target, v) glMultiTexCoord2dvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord2fError(GLenum  target, GLfloat  s, GLfloat  t, const char *filename, int lineno)
{
  glMultiTexCoord2f(target, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2f(target, s, t) glMultiTexCoord2fError(target, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2fvError(GLenum  target, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord2fv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2fv(target, v) glMultiTexCoord2fvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord2iError(GLenum  target, GLint  s, GLint  t, const char *filename, int lineno)
{
  glMultiTexCoord2i(target, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2i(target, s, t) glMultiTexCoord2iError(target, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2ivError(GLenum  target, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord2iv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2iv(target, v) glMultiTexCoord2ivError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord2sError(GLenum  target, GLshort  s, GLshort  t, const char *filename, int lineno)
{
  glMultiTexCoord2s(target, s, t);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2s(target, s, t) glMultiTexCoord2sError(target, s, t, __FILE__, __LINE__)

inline void glMultiTexCoord2svError(GLenum  target, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord2sv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord2sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord2sv(target, v) glMultiTexCoord2svError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord3dError(GLenum  target, GLdouble  s, GLdouble  t, GLdouble  r, const char *filename, int lineno)
{
  glMultiTexCoord3d(target, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3d(target, s, t, r) glMultiTexCoord3dError(target, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3dvError(GLenum  target, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord3dv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3dv(target, v) glMultiTexCoord3dvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord3fError(GLenum  target, GLfloat  s, GLfloat  t, GLfloat  r, const char *filename, int lineno)
{
  glMultiTexCoord3f(target, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3f(target, s, t, r) glMultiTexCoord3fError(target, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3fvError(GLenum  target, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord3fv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3fv(target, v) glMultiTexCoord3fvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord3iError(GLenum  target, GLint  s, GLint  t, GLint  r, const char *filename, int lineno)
{
  glMultiTexCoord3i(target, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3i(target, s, t, r) glMultiTexCoord3iError(target, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3ivError(GLenum  target, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord3iv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3iv(target, v) glMultiTexCoord3ivError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord3sError(GLenum  target, GLshort  s, GLshort  t, GLshort  r, const char *filename, int lineno)
{
  glMultiTexCoord3s(target, s, t, r);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3s(target, s, t, r) glMultiTexCoord3sError(target, s, t, r, __FILE__, __LINE__)

inline void glMultiTexCoord3svError(GLenum  target, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord3sv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord3sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord3sv(target, v) glMultiTexCoord3svError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord4dError(GLenum  target, GLdouble  s, GLdouble  t, GLdouble  r, GLdouble  q, const char *filename, int lineno)
{
  glMultiTexCoord4d(target, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4d]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4d(target, s, t, r, q) glMultiTexCoord4dError(target, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4dvError(GLenum  target, const  GLdouble * v, const char *filename, int lineno)
{
  glMultiTexCoord4dv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4dv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4dv(target, v) glMultiTexCoord4dvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord4fError(GLenum  target, GLfloat  s, GLfloat  t, GLfloat  r, GLfloat  q, const char *filename, int lineno)
{
  glMultiTexCoord4f(target, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4f]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4f(target, s, t, r, q) glMultiTexCoord4fError(target, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4fvError(GLenum  target, const  GLfloat * v, const char *filename, int lineno)
{
  glMultiTexCoord4fv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4fv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4fv(target, v) glMultiTexCoord4fvError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord4iError(GLenum  target, GLint  s, GLint  t, GLint  r, GLint  q, const char *filename, int lineno)
{
  glMultiTexCoord4i(target, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4i]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4i(target, s, t, r, q) glMultiTexCoord4iError(target, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4ivError(GLenum  target, const  GLint * v, const char *filename, int lineno)
{
  glMultiTexCoord4iv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4iv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4iv(target, v) glMultiTexCoord4ivError(target, v, __FILE__, __LINE__)

inline void glMultiTexCoord4sError(GLenum  target, GLshort  s, GLshort  t, GLshort  r, GLshort  q, const char *filename, int lineno)
{
  glMultiTexCoord4s(target, s, t, r, q);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4s]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4s(target, s, t, r, q) glMultiTexCoord4sError(target, s, t, r, q, __FILE__, __LINE__)

inline void glMultiTexCoord4svError(GLenum  target, const  GLshort * v, const char *filename, int lineno)
{
  glMultiTexCoord4sv(target, v);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultiTexCoord4sv]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultiTexCoord4sv(target, v) glMultiTexCoord4svError(target, v, __FILE__, __LINE__)

inline void glLoadTransposeMatrixdError(const  GLdouble  m[16], const char *filename, int lineno)
{
  glLoadTransposeMatrixd(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadTransposeMatrixd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadTransposeMatrixd(m) glLoadTransposeMatrixdError(m, __FILE__, __LINE__)

inline void glLoadTransposeMatrixfError(const  GLfloat  m[16], const char *filename, int lineno)
{
  glLoadTransposeMatrixf(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glLoadTransposeMatrixf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glLoadTransposeMatrixf(m) glLoadTransposeMatrixfError(m, __FILE__, __LINE__)

inline void glMultTransposeMatrixdError(const  GLdouble  m[16], const char *filename, int lineno)
{
  glMultTransposeMatrixd(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultTransposeMatrixd]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultTransposeMatrixd(m) glMultTransposeMatrixdError(m, __FILE__, __LINE__)

inline void glMultTransposeMatrixfError(const  GLfloat  m[16], const char *filename, int lineno)
{
  glMultTransposeMatrixf(m);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glMultTransposeMatrixf]: %s\n", filename, lineno, gluErrorString(error));
}
#define glMultTransposeMatrixf(m) glMultTransposeMatrixfError(m, __FILE__, __LINE__)

inline void glSampleCoverageError(GLclampf  value, GLboolean  invert, const char *filename, int lineno)
{
  glSampleCoverage(value, invert);
  GLenum error = glGetError();
  if (error)
    fprintf(stderr, "%s(%d)[glSampleCoverage]: %s\n", filename, lineno, gluErrorString(error));
}
#define glSampleCoverage(value, invert) glSampleCoverageError(value, invert, __FILE__, __LINE__)

#endif

#endif

#endif

