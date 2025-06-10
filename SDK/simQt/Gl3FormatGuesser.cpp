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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cstdlib>
#include <QOpenGLWindow>
#include "simQt/Gl3FormatGuesser.h"

#ifndef WIN32
#include <X11/Xlib.h>
namespace {
static bool ctxErrorOccurred = false;
int ctxErrorHandler(Display *d, XErrorEvent* e)
{
  // not testing on e->error_code at this point
  ctxErrorOccurred = true;
  return 0;
}
}
#endif

namespace {

/** Simplified replacement for osg::findAsciiToFloat(), to get GLSL version out of a string. Returns 0. if none. */
inline
double extractFirstNumberFrom(const char* value)
{
  if (!value)
    return 0.;
  const std::string& asString = value;
  const auto firstDigit = asString.find_first_of("0123456789");
  if (firstDigit == std::string::npos)
    return 0.;
  return std::atof(value + firstDigit);
}

}

namespace simQt {

Gl3FormatGuesser::Gl3FormatGuesser()
  : format_(QGLFormat::defaultFormat()),
  surfaceFormat_(QSurfaceFormat::defaultFormat())
{
}

void Gl3FormatGuesser::setFormat(const QGLFormat& format)
{
  format_ = format;
}

const QGLFormat& Gl3FormatGuesser::format() const
{
  return format_;
}

int Gl3FormatGuesser::findCompatibleFormat()
{
  QGLFormat candidate = format_;
  if (testFormat_(candidate))
    return 0;

  // Try forcing version to 3.3
  if (candidate.majorVersion() != 3 || candidate.minorVersion() != 3)
  {
    candidate.setVersion(3, 3);
    if (testFormat_(candidate))
    {
      format_ = candidate;
      return 0;
    }
  }

  // Try forcing core profile, if we can get by without FFP
#ifndef OSG_GL_FIXED_FUNCTION_AVAILABLE
  if (candidate.profile() != QGLFormat::CoreProfile)
  {
    candidate.setProfile(QGLFormat::CoreProfile);
    if (testFormat_(candidate))
    {
      format_ = candidate;
      return 0;
    }
  }
#endif

  // Try removing multisampling
  if (candidate.sampleBuffers())
  {
    candidate.setSampleBuffers(false);
    if (testFormat_(candidate))
    {
      format_ = candidate;
      return 0;
    }
  }

  // Try removing stereo if set
  if (candidate.stereo())
  {
    candidate.setStereo(false);
    if (testFormat_(candidate))
    {
      format_ = candidate;
      return 0;
    }
  }

  // No good format found
  return 1;
}

bool Gl3FormatGuesser::testFormat_(const QGLFormat& format) const
{
#ifndef WIN32
  // process any Linux X error as failure to configure a context
  ctxErrorOccurred = false;
  int(*oldHandler)(Display*, XErrorEvent*) =
    XSetErrorHandler(&ctxErrorHandler);

  QGLWidget widget(format);
  if (!ctxErrorOccurred)
    widget.makeCurrent();

  // Restore the original error handler
  XSetErrorHandler(oldHandler);
  if (ctxErrorOccurred)
    return false;
#else
  QGLWidget widget(format);
  widget.makeCurrent();
#endif
  // reset error flag
  while (glGetError() != GL_NO_ERROR) {}
  const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
  if (!glslVersion || glGetError() != GL_NO_ERROR)
    return false;
  return extractFirstNumberFrom(glslVersion) >= 3.3;
}

QGLFormat Gl3FormatGuesser::getFormat(const QGLFormat& format)
{
  Gl3FormatGuesser guesser;
  guesser.setFormat(format);
  guesser.findCompatibleFormat();
  return guesser.format();
}

QGLFormat Gl3FormatGuesser::getFormat()
{
  return Gl3FormatGuesser::getFormat(QGLFormat::defaultFormat());
}

////////////////////////////////////////////////////////////////////

void Gl3FormatGuesser::setSurfaceFormat(const QSurfaceFormat& format)
{
  surfaceFormat_ = format;
}


const QSurfaceFormat& Gl3FormatGuesser::surfaceFormat() const
{
  return surfaceFormat_;
}

int Gl3FormatGuesser::findCompatibleSurfaceFormat()
{
  QSurfaceFormat candidate = surfaceFormat_;
  if (testSurfaceFormat_(candidate))
    return 0;

  // Try forcing version to 3.3
  if (candidate.majorVersion() != 3 || candidate.minorVersion() != 3)
  {
    candidate.setVersion(3, 3);
    if (testSurfaceFormat_(candidate))
    {
      surfaceFormat_ = candidate;
      return 0;
    }
  }

  // Try forcing core profile, if we can get by without FFP
#ifndef OSG_GL_FIXED_FUNCTION_AVAILABLE
  if (candidate.profile() != QSurfaceFormat::CoreProfile)
  {
    candidate.setProfile(QSurfaceFormat::CoreProfile);
    if (testSurfaceFormat_(candidate))
    {
      surfaceFormat_ = candidate;
      return 0;
    }
  }
#endif

  // Try removing multisampling
  if (candidate.samples() > 0)
  {
    candidate.setSamples(0);
    if (testSurfaceFormat_(candidate))
    {
      surfaceFormat_ = candidate;
      return 0;
    }
  }

  // Try removing stereo if set
  if (candidate.stereo())
  {
    candidate.setStereo(false);
    if (testSurfaceFormat_(candidate))
    {
      surfaceFormat_ = candidate;
      return 0;
    }
  }

  // No good format found
  return 1;
}

bool Gl3FormatGuesser::testSurfaceFormat_(const QSurfaceFormat& format) const
{
#ifndef WIN32
  // process any Linux X error as failure to configure a context
  ctxErrorOccurred = false;
  int(*oldHandler)(Display*, XErrorEvent*) =
    XSetErrorHandler(&ctxErrorHandler);

  QOpenGLWindow widget;
  widget.setFormat(format);
  widget.create();
  if (!ctxErrorOccurred)
    widget.makeCurrent();

  // Restore the original error handler
  XSetErrorHandler(oldHandler);
  if (ctxErrorOccurred)
    return false;
#else
  QOpenGLWindow widget;
  widget.setFormat(format);
  widget.create();
  widget.makeCurrent();
#endif

  // reset error flag
  while (glGetError() != GL_NO_ERROR) {}

  const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
  if (!glslVersion || glGetError() != GL_NO_ERROR)
    return false;
  return extractFirstNumberFrom(glslVersion) >= 3.3;
}

QSurfaceFormat Gl3FormatGuesser::getSurfaceFormat(const QSurfaceFormat& format)
{
  Gl3FormatGuesser guesser;
  guesser.setSurfaceFormat(format);
  guesser.findCompatibleSurfaceFormat();
  return guesser.surfaceFormat();
}

QSurfaceFormat Gl3FormatGuesser::getSurfaceFormat()
{
  return Gl3FormatGuesser::getSurfaceFormat(QSurfaceFormat::defaultFormat());
}

}
