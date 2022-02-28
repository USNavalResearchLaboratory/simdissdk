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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/Math"
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

namespace simQt {

Gl3FormatGuesser::Gl3FormatGuesser()
  : format_(QGLFormat::defaultFormat())
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
    candidate.setSampleBuffers(false);
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
  return osg::findAsciiToFloat(glslVersion) >= 3.3f;
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

}
