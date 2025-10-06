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
#ifndef SIMQT_GL3FORMATGUESSER_H
#define SIMQT_GL3FORMATGUESSER_H

#include <QSurfaceFormat>
#if QT_VERSION_MAJOR == 5
#include <QGLFormat>
#endif
#include "simCore/Common/Export.h"

namespace simQt {

/**
 * Different graphics drivers support different capabilities.  SIMDIS has a minimum
 * required OpenGL version of 3.3.  Some context configuration options, such as
 * multisample, have been known to change the OpenGL context version returned by
 * the driver when creating a context.  This class attempts to massage a QGLFormat
 * into one that will return a context that is SIMDIS-compatible without losing
 * configuration options.
 */
class SDKQT_EXPORT Gl3FormatGuesser
{
public:
  /** Default constructor initializes the internal format to QGLFormat::defaultFormat(). */
  Gl3FormatGuesser();

#if QT_VERSION_MAJOR == 5
  /** Change the input format for the findCompatibleFormat() operation. */
  void setFormat(const QGLFormat& format);
  /** Retrieve the guessed format.  findCompatibleFormat() changes this value. */
  const QGLFormat& format() const;

  /**
   * Test the provided QGLFormat for compatibility with SIMDIS given the current
   * hardware configuration.  Values inside the QGLFormat will be altered if the
   * provided QGLFormat is insufficient for SIMDIS.  This method will change the
   * return value provided by format().
   * @return 0 on success; see the return value of format() for the resulting format.
   *   Returns non-zero on error.  This means the class was unable to find a QGLFormat
   *   on this hardware that meets the minimum system requirements for SIMDIS.
   *   Continuing to start the application may lead to software instability or crashes.
   */
  int findCompatibleFormat();

  /** Helper method to return a reasonable format from QGLFormat::defaultFormat(). */
  static QGLFormat getFormat();
  /** Helper method to return a reasonable format from the provided format. */
  static QGLFormat getFormat(const QGLFormat& format);
#endif

  /** Change the input format for the findCompatibleSurfaceFormat() operation. */
  void setSurfaceFormat(const QSurfaceFormat& format);
  /** Retrieve the guessed format. findCompatibleSurfaceFormat() changes this value. */
  const QSurfaceFormat& surfaceFormat() const;

  /** Attempts to find a valid surface format compatible with the configured values. @see findCompatibleFormat() */
  int findCompatibleSurfaceFormat();

  /** Helper method to return a reasonable surface format from QSurfaceFormat::defaultFormat(). */
  static QSurfaceFormat getSurfaceFormat();
  /** Helper method to return a reasonable surface format from the provided format. */
  static QSurfaceFormat getSurfaceFormat(const QSurfaceFormat& format);

private:
#if QT_VERSION_MAJOR == 5
  /** Tests a candidate format for suitability */
  bool testFormat_(const QGLFormat& format) const;

  QGLFormat format_;
#endif

  /** Tests a candidate format for suitability */
  bool testSurfaceFormat_(const QSurfaceFormat& format) const;

  QSurfaceFormat surfaceFormat_;
};

}

#endif /* SIMQT_GL3FORMATGUESSER_H */
